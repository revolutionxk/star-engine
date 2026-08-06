#include "star/rendering/passes/tonemap_pass.hpp"

#include "star/graphics/device.hpp"
#include "star/rendering/render_target.hpp"
#include "star/rendering/viewport.hpp"

namespace star::rendering {
    TonemapPass::TonemapPass(graphics::Device& device, resources::ResourceManager& resources,
                             const PostProcessSettings& settings)
        : PostPass(device, resources, settings, "__tonemap_shader", resources::BuiltinShader::Tonemap) {}

    void TonemapPass::resolve_uniforms(graphics::DeviceContext& gpu) {
        using graphics::UniformType;

        m_tonemap_params = gpu.uniform("u_tonemapParams", UniformType::Vec4);
        m_s_hdr = gpu.uniform("s_hdr", UniformType::Sampler);
        m_s_bloom = gpu.uniform("s_bloom", UniformType::Sampler);
        m_s_ao = gpu.uniform("s_ao", UniformType::Sampler);
    }

    void TonemapPass::render(const RenderContext& ctx) {
        Viewport* viewport = ctx.frame.viewport;
        if (!m_settings.enabled || !viewport)
            return;

        auto& gpu = ctx.gpu;
        if (!ensure_ready(gpu))
            return;

        auto& res = viewport->resources();
        const auto lit = res.texture(resource_names::LIT);
        const auto display_fb = viewport->display_framebuffer();
        if (!lit.is_valid() || !display_fb.is_valid())
            return;

        const auto bloom = res.texture(resource_names::BLOOM);
        const auto ao = res.texture(resource_names::AO);

        const u32 w = viewport->width();
        const u32 h = viewport->height();

        const RenderTarget* ldr =
            m_settings.resolve_needs_fxaa() ? res.target("ldr", w, h, graphics::TextureFormat::RGBA8) : nullptr;

        if (ldr) {
            begin_view(gpu, ctx.view_id, *ldr);
        } else {
            begin_view(gpu, ctx.view_id, display_fb, static_cast<u16>(w), static_cast<u16>(h));
        }

        const f32 bloom_intensity = bloom.is_valid() ? m_settings.bloom_intensity : 0.0f;
        const f32 ao_strength = ao.is_valid() ? m_settings.ssao_strength : 0.0f;
        const Vector4 params{flip_v(), m_settings.exposure, bloom_intensity, ao_strength};
        gpu.set_uniform(m_post_params, &params);

        const Vector4 tonemap_params{static_cast<f32>(m_settings.tonemap), 0.0f, 0.0f, 0.0f};
        gpu.set_uniform(m_tonemap_params, &tonemap_params);

        gpu.set_texture(m_s_hdr, 0, lit);
        gpu.set_texture(m_s_bloom, 1, bloom.is_valid() ? bloom : lit);
        gpu.set_texture(m_s_ao, 2, ao.is_valid() ? ao : lit);
        draw_fullscreen(gpu, ctx.view_id);

        if (ldr) {
            res.publish(resource_names::LDR, ldr->color_texture(0));
        }
    }

    FxaaPass::FxaaPass(graphics::Device& device, resources::ResourceManager& resources,
                       const PostProcessSettings& settings)
        : PostPass(device, resources, settings, "__fxaa_shader", resources::BuiltinShader::Fxaa) {}

    void FxaaPass::resolve_uniforms(graphics::DeviceContext& gpu) {
        m_fxaa_params = gpu.uniform("u_fxaaParams", graphics::UniformType::Vec4);
        m_s_src = gpu.uniform("s_src", graphics::UniformType::Sampler);
    }

    void FxaaPass::render(const RenderContext& ctx) {
        Viewport* viewport = ctx.frame.viewport;
        if (!m_settings.resolve_needs_fxaa() || !viewport)
            return;

        auto& gpu = ctx.gpu;
        if (!ensure_ready(gpu))
            return;

        auto& res = viewport->resources();
        const auto ldr = res.texture(resource_names::LDR);
        const auto display_fb = viewport->display_framebuffer();
        if (!ldr.is_valid() || !display_fb.is_valid())
            return;

        const u32 w = viewport->width();
        const u32 h = viewport->height();

        begin_view(gpu, ctx.view_id, display_fb, static_cast<u16>(w), static_cast<u16>(h));

        const Vector4 flip_params{flip_v(), 0.0f, 0.0f, 0.0f};
        gpu.set_uniform(m_post_params, &flip_params);
        const Vector4 fxaa_params{1.0f / static_cast<f32>(w), 1.0f / static_cast<f32>(h), 0.0f, 0.0f};
        gpu.set_uniform(m_fxaa_params, &fxaa_params);
        gpu.set_texture(m_s_src, 0, ldr);
        draw_fullscreen(gpu, ctx.view_id);
    }
} // namespace star::rendering
