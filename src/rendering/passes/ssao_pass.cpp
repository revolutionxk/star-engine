#include "star/rendering/passes/ssao_pass.hpp"

#include "star/graphics/device.hpp"
#include "star/rendering/render_target.hpp"
#include "star/rendering/viewport.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/shader.hpp"

namespace star::rendering {
    SsaoPass::SsaoPass(graphics::Device& device, resources::ResourceManager& resources,
                       const PostProcessSettings& settings)
        : PostPass(device, resources, settings, "__ssao_shader", resources::BuiltinShader::Ssao) {}

    void SsaoPass::resolve_uniforms(graphics::DeviceContext& gpu) {
        using graphics::UniformType;

        m_inv_proj = gpu.uniform("u_ssaoInvProj", UniformType::Mat4);
        m_proj = gpu.uniform("u_ssaoProj", UniformType::Mat4);
        m_params = gpu.uniform("u_ssaoParams", UniformType::Vec4);
        m_texel = gpu.uniform("u_ssaoTexel", UniformType::Vec4);
        m_blur_params = gpu.uniform("u_ssaoBlurParams", UniformType::Vec4);
        m_s_depth = gpu.uniform("s_depth", UniformType::Sampler);
        m_s_gbuffer = gpu.uniform("s_gbuffer", UniformType::Sampler);
        m_s_ao = gpu.uniform("s_ao", UniformType::Sampler);
    }

    void SsaoPass::render(const RenderContext& ctx) {
        Viewport* viewport = ctx.frame.viewport;
        if (!m_settings.enabled || !m_settings.ssao_enabled || !viewport)
            return;

        auto& gpu = ctx.gpu;
        if (!ensure_ready(gpu))
            return;

        if (!m_blur_shader.is_valid()) {
            const auto res = m_resources.register_builtin_shader("__ssao_blur_shader", resources::BuiltinShader::SsaoBlur);
            if (const auto* shader = m_resources.get_shader(res))
                m_blur_shader = shader->handle;
        }
        if (!m_blur_shader.is_valid())
            return;

        auto& res = viewport->resources();
        const auto depth = res.texture(resource_names::DEPTH);
        const auto normal = res.texture(resource_names::NORMAL);
        if (!depth.is_valid())
            return;

        const u32 w = viewport->width();
        const u32 h = viewport->height();
        RenderTarget* raw = res.target("ssao_raw", w, h, graphics::TextureFormat::RGBA8);
        RenderTarget* blurred = res.target("ssao_blurred", w, h, graphics::TextureFormat::RGBA8);
        if (!raw || !blurred)
            return;

        const Matrix4 proj = viewport->projection_matrix();
        const Matrix4 inv_proj = Matrix4::inverse(proj);
        const Vector4 no_flip{0.0f, 0.0f, 0.0f, 0.0f};
        const Vector4 texel{1.0f / static_cast<f32>(w), 1.0f / static_cast<f32>(h), flip_v(), 0.0f};

        const u32 occlusion_view = ctx.view_id;
        begin_view(gpu, occlusion_view, *raw);
        gpu.set_uniform(m_post_params, &no_flip);
        gpu.set_uniform(m_inv_proj, &inv_proj);
        gpu.set_uniform(m_proj, &proj);
        const Vector4 params{m_settings.ssao_radius, m_settings.ssao_power, m_settings.ssao_fade, 0.0f};
        gpu.set_uniform(m_params, &params);
        gpu.set_uniform(m_texel, &texel);
        gpu.set_texture(m_s_depth, 0, depth);
        gpu.set_texture(m_s_gbuffer, 1, normal);
        draw_fullscreen(gpu, occlusion_view);

        const u32 blur_view = ctx.frame.views.acquire("SsaoBlur");
        begin_view(gpu, blur_view, *blurred);
        gpu.set_uniform(m_post_params, &no_flip);
        const Vector4 blur{1.0f / static_cast<f32>(w), 1.0f / static_cast<f32>(h), 5000.0f, 0.0f};
        gpu.set_uniform(m_blur_params, &blur);
        gpu.set_texture(m_s_ao, 0, raw->color_texture(0));
        gpu.set_texture(m_s_depth, 1, depth);
        draw_fullscreen(gpu, blur_view, m_blur_shader);

        res.publish(resource_names::AO, blurred->color_texture(0));
    }
} // namespace star::rendering
