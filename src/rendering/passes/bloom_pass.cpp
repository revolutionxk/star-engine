#include "star/rendering/passes/bloom_pass.hpp"

#include <algorithm>

#include "star/graphics/device.hpp"
#include "star/rendering/render_target.hpp"
#include "star/rendering/viewport.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/shader.hpp"

namespace star::rendering {
    BloomPass::BloomPass(graphics::Device& device, resources::ResourceManager& resources,
                         const PostProcessSettings& settings)
        : PostPass(device, resources, settings, "__bloom_bright_shader", resources::BuiltinShader::BloomBright) {}

    void BloomPass::resolve_uniforms(graphics::DeviceContext& gpu) {
        using graphics::UniformType;

        m_bloom_params = gpu.uniform("u_bloomParams", UniformType::Vec4);
        m_blur_params = gpu.uniform("u_blurParams", UniformType::Vec4);
        m_s_hdr = gpu.uniform("s_hdr", UniformType::Sampler);
        m_s_src = gpu.uniform("s_src", UniformType::Sampler);
    }

    void BloomPass::render(const RenderContext& ctx) {
        Viewport* viewport = ctx.frame.viewport;
        if (!m_settings.enabled || !m_settings.bloom_enabled || !viewport)
            return;

        auto& gpu = ctx.gpu;
        if (!ensure_ready(gpu))
            return;

        if (!m_blur_shader.is_valid()) {
            const auto res = m_resources.register_builtin_shader("__bloom_blur_shader", resources::BuiltinShader::BloomBlur);
            if (const auto* shader = m_resources.get_shader(res))
                m_blur_shader = shader->handle;
        }
        if (!m_blur_shader.is_valid())
            return;

        auto& res = viewport->resources();
        const auto lit = res.texture(resource_names::LIT);
        if (!lit.is_valid())
            return;

        const u32 w = std::max(1u, viewport->width() / 2);
        const u32 h = std::max(1u, viewport->height() / 2);
        RenderTarget* a = res.target("bloom_a", w, h, graphics::TextureFormat::RGBA16F);
        RenderTarget* b = res.target("bloom_b", w, h, graphics::TextureFormat::RGBA16F);
        if (!a || !b)
            return;

        const Vector4 flip_only{flip_v(), 0.0f, 0.0f, 0.0f};

        const u32 bright_view = ctx.view_id;
        begin_view(gpu, bright_view, *a);
        gpu.set_uniform(m_post_params, &flip_only);
        const Vector4 bloom_params{m_settings.bloom_threshold, m_settings.bloom_knee, 0.0f, 0.0f};
        gpu.set_uniform(m_bloom_params, &bloom_params);
        gpu.set_texture(m_s_hdr, 0, lit);
        draw_fullscreen(gpu, bright_view);

        const u32 blur_h_view = ctx.frame.views.acquire("BloomBlurH");
        begin_view(gpu, blur_h_view, *b);
        gpu.set_uniform(m_post_params, &flip_only);
        const Vector4 blur_h{1.0f / static_cast<f32>(w), 0.0f, 0.0f, 0.0f};
        gpu.set_uniform(m_blur_params, &blur_h);
        gpu.set_texture(m_s_src, 0, a->color_texture(0));
        draw_fullscreen(gpu, blur_h_view, m_blur_shader);

        const u32 blur_v_view = ctx.frame.views.acquire("BloomBlurV");
        begin_view(gpu, blur_v_view, *a);
        gpu.set_uniform(m_post_params, &flip_only);
        const Vector4 blur_v{0.0f, 1.0f / static_cast<f32>(h), 0.0f, 0.0f};
        gpu.set_uniform(m_blur_params, &blur_v);
        gpu.set_texture(m_s_src, 0, b->color_texture(0));
        draw_fullscreen(gpu, blur_v_view, m_blur_shader);

        res.publish(resource_names::BLOOM, a->color_texture(0));
    }
} // namespace star::rendering
