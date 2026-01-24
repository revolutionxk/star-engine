#include "imgui_bgfx_renderer.hpp"

#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <imgui.h>

#include "star/core/common.hpp"
#include "star/core/logger.hpp"
#include "star/resources/shader/shaders.hpp"

namespace star::graphics {

    ImGuiBGFXRenderer::~ImGuiBGFXRenderer() {
        shutdown();
    }

    bool ImGuiBGFXRenderer::initialize() {
        if (m_initialized) {
            STAR_LOG_WARN(LogCategory::Graphics, "ImGuiBgfxRenderer already initialized");
            return true;
        }

        STAR_LOG_INFO(LogCategory::Graphics, "Initializing ImGui BGFX renderer...");

        m_data.vertex_layout.begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();

        const bgfx::EmbeddedShader embedded_shaders[] = {k_imgui_vs, k_imgui_fs, BGFX_EMBEDDED_SHADER_END()};

        const auto renderer_type = bgfx::getRendererType();
        const auto vs = bgfx::createEmbeddedShader(embedded_shaders, renderer_type, "v_imgui");
        const auto fs = bgfx::createEmbeddedShader(embedded_shaders, renderer_type, "f_imgui");

        if (!bgfx::isValid(vs) || !bgfx::isValid(fs)) {
            STAR_LOG_ERROR(LogCategory::Graphics, "Failed to create ImGui shaders");
            return false;
        }

        m_data.shader_program = bgfx::createProgram(vs, fs, true);
        if (!bgfx::isValid(m_data.shader_program)) {
            STAR_LOG_ERROR(LogCategory::Graphics, "Failed to create ImGui shader program");
            return false;
        }

        m_data.s_tex = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

        const auto& io = ImGui::GetIO();
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        m_data.font_texture =
            bgfx::createTexture2D(static_cast<uint16_t>(width), static_cast<uint16_t>(height), false, 1,
                                  bgfx::TextureFormat::BGRA8, 0, bgfx::copy(pixels, width * height * 4));

        if (!bgfx::isValid(m_data.font_texture)) {
            STAR_LOG_ERROR(LogCategory::Graphics, "Failed to create ImGui font texture");
            return false;
        }

        io.Fonts->SetTexID(reinterpret_cast<void*>(static_cast<uintptr_t>(m_data.font_texture.idx)));

        m_initialized = true;
        STAR_LOG_INFO(LogCategory::Graphics, "ImGui BGFX renderer initialized successfully");

        return true;
    }

    void ImGuiBGFXRenderer::shutdown() {
        if (!m_initialized) {
            return;
        }

        STAR_LOG_INFO(LogCategory::Graphics, "Shutting down ImGui BGFX renderer...");

        if (bgfx::isValid(m_data.shader_program)) {
            bgfx::destroy(m_data.shader_program);
            m_data.shader_program = BGFX_INVALID_HANDLE;
        }

        if (bgfx::isValid(m_data.font_texture)) {
            bgfx::destroy(m_data.font_texture);
            m_data.font_texture = BGFX_INVALID_HANDLE;
        }

        if (bgfx::isValid(m_data.s_tex)) {
            bgfx::destroy(m_data.s_tex);
            m_data.s_tex = BGFX_INVALID_HANDLE;
        }

        m_initialized = false;
    }

    void ImGuiBGFXRenderer::render(const u32 view_id) {
        if (!m_initialized) {
            return;
        }

        ImDrawData* draw_data = ImGui::GetDrawData();
        if (!draw_data || draw_data->TotalVtxCount == 0) {
            return;
        }

        if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f) {
            return;
        }

        const float width = draw_data->DisplaySize.x;
        const float height = draw_data->DisplaySize.y;

        bgfx::setViewName(view_id, "ImGui");
        bgfx::setViewMode(view_id, bgfx::ViewMode::Sequential);

        {
            const bgfx::Caps* caps = bgfx::getCaps();
            float ortho[16];
            const float x = draw_data->DisplayPos.x;
            const float y = draw_data->DisplayPos.y;
            const float width_val = draw_data->DisplaySize.x;
            const float height_val = draw_data->DisplaySize.y;

            bx::mtxOrtho(ortho, x, x + width_val, y + height_val, y, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
            bgfx::setViewTransform(view_id, nullptr, ortho);
            bgfx::setViewRect(view_id, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
        }

        const auto clip_pos = draw_data->DisplayPos;
        const auto clip_scale = draw_data->FramebufferScale;

        for (int32_t ii = 0, num = draw_data->CmdListsCount; ii < num; ++ii) {
            const auto* cmd_list = draw_data->CmdLists[ii];

            const auto num_vertices = static_cast<uint32_t>(cmd_list->VtxBuffer.Size);
            const auto num_indices = static_cast<uint32_t>(cmd_list->IdxBuffer.Size);

            if (num_vertices != bgfx::getAvailTransientVertexBuffer(num_vertices, m_data.vertex_layout) ||
                num_indices != bgfx::getAvailTransientIndexBuffer(num_indices)) {
                break;
            }

            bgfx::TransientVertexBuffer vertex_buffer{};
            bgfx::TransientIndexBuffer index_buffer{};

            bgfx::allocTransientVertexBuffer(&vertex_buffer, num_vertices, m_data.vertex_layout);
            bgfx::allocTransientIndexBuffer(&index_buffer, num_indices, sizeof(ImDrawIdx) == 4);

            auto* verts = reinterpret_cast<ImDrawVert*>(vertex_buffer.data);
            memcpy(verts, cmd_list->VtxBuffer.Data, num_vertices * sizeof(ImDrawVert));

            auto* indices = reinterpret_cast<ImDrawIdx*>(index_buffer.data);
            memcpy(indices, cmd_list->IdxBuffer.Data, num_indices * sizeof(ImDrawIdx));

            bgfx::Encoder* encoder = bgfx::begin();

            for (const ImDrawCmd& cmd : cmd_list->CmdBuffer) {
                if (cmd.UserCallback) {
                    cmd.UserCallback(cmd_list, &cmd);
                } else if (0 != cmd.ElemCount) {
                    constexpr auto state =
                        0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA |
                        BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);

                    bgfx::TextureHandle texture = {static_cast<uint16_t>(cmd.GetTexID())};

                    if (!bgfx::isValid(texture)) {
                        texture = m_data.font_texture;
                    }

                    const uint16_t xx =
                        static_cast<uint16_t>(bx::max(cmd.ClipRect.x - clip_pos.x, 0.0f) * clip_scale.x);
                    const uint16_t yy =
                        static_cast<uint16_t>(bx::max(cmd.ClipRect.y - clip_pos.y, 0.0f) * clip_scale.y);
                    const uint16_t ww =
                        static_cast<uint16_t>(bx::min(cmd.ClipRect.z - clip_pos.x, 65535.0f) * clip_scale.x) - xx;
                    const uint16_t hh =
                        static_cast<uint16_t>(bx::min(cmd.ClipRect.w - clip_pos.y, 65535.0f) * clip_scale.y) - yy;

                    encoder->setScissor(xx, yy, ww, hh);
                    encoder->setState(state);
                    encoder->setTexture(0, m_data.s_tex, texture);
                    encoder->setVertexBuffer(0, &vertex_buffer, cmd.VtxOffset, num_vertices);
                    encoder->setIndexBuffer(&index_buffer, cmd.IdxOffset, cmd.ElemCount);
                    encoder->submit(view_id, m_data.shader_program);
                }
            }

            bgfx::end(encoder);
        }
    }

    void ImGuiBGFXRenderer::reset(const u32 width, const u32 height) {}

} // namespace star::graphics
