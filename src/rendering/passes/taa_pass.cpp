#include "star/rendering/passes/taa_pass.hpp"

#include "star/graphics/device.hpp"
#include "star/rendering/render_target.hpp"
#include "star/rendering/viewport.hpp"

namespace star::rendering {
    TaaPass::TaaPass(graphics::Device& device, resources::ResourceManager& resources,
                     const PostProcessSettings& settings)
        : PostPass(device, resources, settings, "__taa_shader", resources::BuiltinShader::Taa) {}

    void TaaPass::resolve_uniforms(graphics::DeviceContext& gpu) {
        using graphics::UniformType;

        m_cur_inv_vp = gpu.uniform("u_taaCurInvVP", UniformType::Mat4);
        m_prev_vp = gpu.uniform("u_taaPrevVP", UniformType::Mat4);
        m_params = gpu.uniform("u_taaParams", UniformType::Vec4);
        m_texel = gpu.uniform("u_taaTexel", UniformType::Vec4);
        m_s_current = gpu.uniform("s_current", UniformType::Sampler);
        m_s_history = gpu.uniform("s_history", UniformType::Sampler);
        m_s_depth = gpu.uniform("s_depth", UniformType::Sampler);
        m_s_velocity = gpu.uniform("s_velocity", UniformType::Sampler);
    }

    void TaaPass::render(const RenderContext& ctx) {
        Viewport* viewport = ctx.frame.viewport;
        if (!m_settings.enabled || !m_settings.taa_enabled || !viewport)
            return;

        auto& gpu = ctx.gpu;
        if (!ensure_ready(gpu))
            return;

        auto& res = viewport->resources();
        const auto lit = res.texture(resource_names::LIT);
        const auto depth = res.texture(resource_names::DEPTH);
        const auto velocity = res.texture(resource_names::VELOCITY);
        if (!lit.is_valid() || !depth.is_valid())
            return;

        const u32 w = viewport->width();
        const u32 h = viewport->height();
        RenderTarget* a = res.target("taa_a", w, h, graphics::TextureFormat::RGBA16F);
        RenderTarget* b = res.target("taa_b", w, h, graphics::TextureFormat::RGBA16F);
        if (!a || !b)
            return;

        u64& frames = res.counter("taa_frames");
        const bool even = (frames % 2) == 0;
        RenderTarget* out = even ? a : b;
        const RenderTarget* history = even ? b : a;

        const f32 flip = flip_v();
        const Vector4 flip_only{flip, 0.0f, 0.0f, 0.0f};
        const Matrix4 cur_inv_vp = Matrix4::inverse(viewport->cur_view_proj());
        const Matrix4& prev_vp = viewport->prev_view_proj();

        begin_view(gpu, ctx.view_id, *out);
        gpu.set_uniform(m_post_params, &flip_only);
        gpu.set_uniform(m_cur_inv_vp, &cur_inv_vp);
        gpu.set_uniform(m_prev_vp, &prev_vp);
        const Vector4 params{flip, m_settings.taa_blend, frames > 0 ? 1.0f : 0.0f, 0.0f};
        gpu.set_uniform(m_params, &params);
        const Vector4 texel{1.0f / static_cast<f32>(w), 1.0f / static_cast<f32>(h), 0.0f, 0.0f};
        gpu.set_uniform(m_texel, &texel);
        gpu.set_texture(m_s_current, 0, lit);
        gpu.set_texture(m_s_history, 1, history->color_texture(0));
        gpu.set_texture(m_s_depth, 2, depth);
        gpu.set_texture(m_s_velocity, 3, velocity);
        draw_fullscreen(gpu, ctx.view_id);

        res.publish(resource_names::LIT, out->color_texture(0));
        ++frames;
    }
} // namespace star::rendering
