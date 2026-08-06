#include "star/rendering/passes/picking_pass.hpp"

#include <algorithm>

#include <bgfx/bgfx.h>

#include "star/core/logger.hpp"
#include "star/graphics/device.hpp"
#include "star/graphics/device_context.hpp"
#include "star/rendering/render_scene.hpp"
#include "star/rendering/viewport.hpp"
#include "star/resources/mesh/mesh.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/shader.hpp"

namespace star::rendering {
    PickingPass::PickingPass(graphics::Device& device, resources::ResourceManager& resources)
        : m_device(device), m_resources(resources) {}

    PickingPass::~PickingPass() = default;

    Vector4 PickingPass::encode_id(const u32 id) {
        return Vector4{static_cast<f32>(id & 0xFFu) / 255.0f, static_cast<f32>((id >> 8) & 0xFFu) / 255.0f,
                       static_cast<f32>((id >> 16) & 0xFFu) / 255.0f, 1.0f};
    }

    void PickingPass::request(const u32 pixel_x, const u32 pixel_y) {
        m_pixel_x = pixel_x;
        m_pixel_y = pixel_y;
        m_request_pending = true;
    }

    void PickingPass::render(const RenderContext& ctx) {
        m_request_pending = false;

        const Viewport* viewport = ctx.frame.viewport;
        const RenderScene* scene = ctx.frame.scene;
        if (!viewport || !scene)
            return;

        if (!m_pick_shader.is_valid()) {
            const auto shader_res =
                m_resources.register_builtin_shader("__pick_shader", resources::BuiltinShader::Picking);
            if (const auto* shader = m_resources.get_shader(shader_res))
                m_pick_shader = shader->handle;
        }
        if (!m_pick_shader.is_valid())
            return;

        const u32 width = viewport->width();
        const u32 height = viewport->height();
        if (width == 0 || height == 0)
            return;

        if (!m_target.is_valid() || m_target.width() != width || m_target.height() != height)
            m_target.create(&m_device, width, height, graphics::TextureFormat::RGBA8, true);
        if (!m_readback.is_valid())
            m_readback = m_device.create_readback_texture(1, 1);
        if (!m_target.is_valid() || !m_readback.is_valid())
            return;

        auto& gpu = ctx.gpu;

        if (!m_pick_id_uniform.is_valid())
            m_pick_id_uniform = gpu.uniform("u_pickId", graphics::UniformType::Vec4);

        constexpr u32 view = 240;
        gpu.set_view_framebuffer(view, m_target.framebuffer());
        gpu.set_view_rect(view, 0, 0, static_cast<u16>(width), static_cast<u16>(height));
        gpu.set_view_clear(view, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0);
        gpu.set_view_transform(view, viewport->view_matrix(), viewport->projection_matrix());

        m_ids.clear();
        u32 index = 1;
        for (const auto& renderable : scene->renderables) {
            if (!renderable.mesh.is_valid())
                continue;
            const auto* mesh = m_resources.get_mesh(renderable.mesh);
            if (!mesh || !mesh->vertex_buffer.is_valid() || !mesh->index_buffer.is_valid())
                continue;

            gpu.set_transform(renderable.model_matrix);
            gpu.set_vertex_buffer(0, mesh->vertex_buffer);
            gpu.set_index_buffer(mesh->index_buffer);

            const Vector4 id_color = encode_id(index);
            gpu.set_uniform(m_pick_id_uniform, &id_color);
            gpu.submit(view, m_pick_shader);

            m_ids.push_back(renderable.entity_id);
            ++index;
        }

        const u16 px = static_cast<u16>(std::min(m_pixel_x, width - 1));
        const u16 py = static_cast<u16>(std::min(m_pixel_y, height - 1));

        constexpr u32 blit_view = 241;
        gpu.blit(blit_view, m_readback, 0, 0, m_target.color_texture(0), px, py, 1, 1);
        m_ready_frame = gpu.read_texture(m_readback, m_readback_pixel);
        m_awaiting_readback = true;
    }

    std::optional<u64> PickingPass::poll() {
        if (!m_awaiting_readback)
            return std::nullopt;

        const auto* context = m_device.context();
        if (!context || context->current_frame() < m_ready_frame)
            return std::nullopt;

        m_awaiting_readback = false;

        const u32 index = static_cast<u32>(m_readback_pixel[0]) | (static_cast<u32>(m_readback_pixel[1]) << 8) |
                          (static_cast<u32>(m_readback_pixel[2]) << 16);
        if (index == 0 || index > m_ids.size())
            return u64{0};
        return m_ids[index - 1];
    }
} // namespace star::rendering
