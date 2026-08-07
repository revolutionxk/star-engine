#include "star/rendering/passes/post_pass.hpp"

#include "star/graphics/device.hpp"
#include "star/graphics/pipeline_state.hpp"
#include "star/rendering/render_target.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/shader.hpp"

namespace star::rendering {
    PostPass::PostPass(graphics::Device& device, resources::ResourceManager& resources,
                       const PostProcessSettings& settings, const char* shader_name,
                       const resources::BuiltinShader shader_id)
        : m_device(device), m_resources(resources), m_settings(settings), m_shader_name(shader_name),
          m_shader_id(shader_id) {}

    bool PostPass::ensure_ready(graphics::DeviceContext& gpu) {
        if (!m_shader.is_valid()) {
            if (const auto res = m_resources.register_builtin_shader(m_shader_name, m_shader_id); res.is_valid()) {
                if (const auto* shader = m_resources.get_shader(res)) {
                    m_shader = shader->handle;
                }
            }
        }
        if (!m_shader.is_valid()) {
            return false;
        }

        if (!m_uniforms_resolved) {
            m_post_params = gpu.uniform("u_postParams", graphics::UniformType::Vec4);
            resolve_uniforms(gpu);
            m_uniforms_resolved = true;
        }
        return true;
    }

    void PostPass::begin_view(graphics::DeviceContext& gpu, const u32 view_id, const RenderTarget& target) const {
        begin_view(gpu, view_id, target.framebuffer(), static_cast<u16>(target.width()),
                   static_cast<u16>(target.height()));
    }

    void PostPass::begin_view(graphics::DeviceContext& gpu, const u32 view_id,
                              const graphics::ResourceHandle<graphics::Framebuffer> framebuffer, const u16 width,
                              const u16 height) const {
        gpu.set_view_framebuffer(view_id, framebuffer);
        gpu.set_view_rect(view_id, 0, 0, width, height);
        gpu.set_view_clear(view_id, graphics::ClearFlags::None, 0, 1.0f, 0);
    }

    void PostPass::draw_fullscreen(graphics::DeviceContext& gpu, const u32 view_id) const {
        draw_fullscreen(gpu, view_id, m_shader);
    }

    void PostPass::draw_fullscreen(graphics::DeviceContext& gpu, const u32 view_id,
                                   const graphics::ResourceHandle<graphics::Shader> shader,
                                   const graphics::BlendMode blend) {
        static constexpr f32 k_tri[6] = {-1.0f, -1.0f, 3.0f, -1.0f, -1.0f, 3.0f};
        const graphics::PipelineState state{
            .blend_mode = blend,
            .cull = graphics::CullMode::None,
            .depth_test = graphics::DepthTest::None,
            .depth_write = false,
        };
        gpu.set_pipeline_state(state);
        gpu.set_transient_vertex_buffer(0, k_tri, 3, graphics::VertexLayoutType::ScreenPos);
        gpu.submit(view_id, shader);
    }

    f32 PostPass::flip_v() const {
        return m_device.caps().origin_bottom_left ? 0.0f : 1.0f;
    }
} // namespace star::rendering
