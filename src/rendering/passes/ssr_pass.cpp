#include "star/rendering/passes/ssr_pass.hpp"

#include "star/graphics/device.hpp"
#include "star/rendering/render_target.hpp"
#include "star/rendering/viewport.hpp"

namespace star::rendering {
    SsrPass::SsrPass(graphics::Device& device, resources::ResourceManager& resources,
                     const PostProcessSettings& settings)
        : PostPass(device, resources, settings, "__ssr_shader", resources::BuiltinShader::Ssr) {}

    void SsrPass::resolve_uniforms(graphics::DeviceContext& gpu) {
        using graphics::UniformType;

        m_proj = gpu.uniform("u_ssrProj", UniformType::Mat4);
        m_inv_proj = gpu.uniform("u_ssrInvProj", UniformType::Mat4);
        m_params = gpu.uniform("u_ssrParams", UniformType::Vec4);
        m_texel = gpu.uniform("u_ssrTexel", UniformType::Vec4);
        m_s_hdr = gpu.uniform("s_hdr", UniformType::Sampler);
        m_s_depth = gpu.uniform("s_depth", UniformType::Sampler);
        m_s_scene = gpu.uniform("s_scene", UniformType::Sampler);
        m_s_gbuffer = gpu.uniform("s_gbuffer", UniformType::Sampler);
    }

    void SsrPass::render(const RenderContext& ctx) {
        Viewport* viewport = ctx.frame.viewport;
        if (!m_settings.enabled || !m_settings.ssr_enabled || !viewport)
            return;

        auto& gpu = ctx.gpu;
        if (!ensure_ready(gpu))
            return;

        auto& res = viewport->resources();
        const auto lit = res.texture(resource_names::LIT);
        const auto depth = res.texture(resource_names::DEPTH);
        const auto normal = res.texture(resource_names::NORMAL);
        if (!lit.is_valid() || !depth.is_valid())
            return;

        const u32 w = viewport->width();
        const u32 h = viewport->height();
        RenderTarget* march = res.target("ssr_march", w, h, graphics::TextureFormat::RGBA16F);
        RenderTarget* resolve = res.target("ssr_resolve", w, h, graphics::TextureFormat::RGBA16F);
        if (!march || !resolve)
            return;

        const f32 flip = flip_v();
        const Vector4 flip_only{flip, 0.0f, 0.0f, 0.0f};
        const Matrix4 proj = viewport->projection_matrix();
        const Matrix4 inv_proj = Matrix4::inverse(proj);

        gpu.set_uniform(m_proj, &proj);
        gpu.set_uniform(m_inv_proj, &inv_proj);
        const Vector4 params{m_settings.ssr_max_distance, m_settings.ssr_thickness, 64.0f, m_settings.ssr_intensity};
        gpu.set_uniform(m_params, &params);

        const u32 march_view = ctx.view_id;
        begin_view(gpu, march_view, *march);
        gpu.set_uniform(m_post_params, &flip_only);
        const Vector4 march_texel{1.0f / static_cast<f32>(w), 1.0f / static_cast<f32>(h), flip, 0.0f};
        gpu.set_uniform(m_texel, &march_texel);
        gpu.set_texture(m_s_hdr, 0, lit);
        gpu.set_texture(m_s_depth, 1, depth);
        gpu.set_texture(m_s_gbuffer, 3, normal);
        draw_fullscreen(gpu, march_view);

        const u32 resolve_view = ctx.frame.views.acquire("SsrResolve");
        begin_view(gpu, resolve_view, *resolve);
        gpu.set_uniform(m_post_params, &flip_only);
        const Vector4 resolve_texel{1.0f / static_cast<f32>(w), 1.0f / static_cast<f32>(h), flip, 1.0f};
        gpu.set_uniform(m_texel, &resolve_texel);
        gpu.set_texture(m_s_hdr, 0, march->color_texture(0));
        gpu.set_texture(m_s_depth, 1, depth);
        gpu.set_texture(m_s_scene, 2, lit);
        gpu.set_texture(m_s_gbuffer, 3, normal);
        draw_fullscreen(gpu, resolve_view);

        res.publish(resource_names::LIT, resolve->color_texture(0));
    }
} // namespace star::rendering
