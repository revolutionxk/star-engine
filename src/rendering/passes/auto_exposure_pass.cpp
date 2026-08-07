#include "star/rendering/passes/auto_exposure_pass.hpp"

#include <algorithm>
#include <cmath>

#include "star/graphics/device.hpp"
#include "star/rendering/render_target.hpp"
#include "star/rendering/viewport.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/shader.hpp"

namespace star::rendering {
    AutoExposurePass::AutoExposurePass(graphics::Device& device, resources::ResourceManager& resources,
                                       const PostProcessSettings& settings)
        : PostPass(device, resources, settings, "__lum_reduce_shader", resources::BuiltinShader::LumReduce) {}

    void AutoExposurePass::resolve_uniforms(graphics::DeviceContext& gpu) {
        using graphics::UniformType;

        m_lum_params = gpu.uniform("u_lumParams", UniformType::Vec4);
        m_exposure_params = gpu.uniform("u_exposureParams", UniformType::Vec4);
        m_s_src = gpu.uniform("s_src", UniformType::Sampler);
        m_s_lum = gpu.uniform("s_lum", UniformType::Sampler);
        m_s_prev = gpu.uniform("s_prev", UniformType::Sampler);
    }

    void AutoExposurePass::render(const RenderContext& ctx) {
        Viewport* viewport = ctx.frame.viewport;
        if (!m_settings.enabled || !m_settings.auto_exposure_enabled || !viewport)
            return;

        auto& gpu = ctx.gpu;
        if (!ensure_ready(gpu))
            return;

        if (!m_exposure_shader.is_valid()) {
            const auto res = m_resources.register_builtin_shader("__exposure_shader", resources::BuiltinShader::Exposure);
            if (const auto* shader = m_resources.get_shader(res))
                m_exposure_shader = shader->handle;
        }
        if (!m_exposure_shader.is_valid())
            return;

        auto& res = viewport->resources();
        const auto lit = res.texture(resource_names::LIT);
        if (!lit.is_valid())
            return;

        const Vector4 flip_only{flip_v(), 0.0f, 0.0f, 0.0f};

        graphics::ResourceHandle<graphics::Texture> src = lit;
        u32 size = LUM_SIZE;
        char name[16];

        for (u32 step = 0; step <= REDUCE_STEPS; ++step) {
            std::snprintf(name, sizeof(name), "lum_%u", step);
            RenderTarget* target = res.target(name, size, size, graphics::TextureFormat::RGBA16F);
            if (!target)
                return;

            const u32 view = ctx.frame.views.acquire("LumReduce");
            begin_view(gpu, view, *target);
            gpu.set_uniform(m_post_params, &flip_only);

            const f32 spacing = 1.0f / static_cast<f32>(size * REDUCE_FACTOR);
            const Vector4 params{step == 0 ? 1.0f : 0.0f, spacing, spacing, 0.0f};
            gpu.set_uniform(m_lum_params, &params);
            gpu.set_texture(m_s_src, 0, src);
            draw_fullscreen(gpu, view);

            src = target->color_texture(0);
            size /= REDUCE_FACTOR;
        }

        RenderTarget* a = res.target("exposure_a", 1, 1, graphics::TextureFormat::RGBA16F);
        RenderTarget* b = res.target("exposure_b", 1, 1, graphics::TextureFormat::RGBA16F);
        if (!a || !b)
            return;

        u64& frames = res.counter("exposure_frames");
        const bool even = (frames % 2) == 0;
        RenderTarget* out = even ? a : b;
        const RenderTarget* prev = even ? b : a;

        const f32 speed = std::max(m_settings.exposure_speed, 0.0f);
        const f32 blend = 1.0f - std::exp(-ctx.frame.delta_time * speed);

        const u32 view = ctx.frame.views.acquire("Exposure");
        begin_view(gpu, view, *out);
        gpu.set_uniform(m_post_params, &flip_only);

        const Vector4 params{m_settings.exposure_min_ev, m_settings.exposure_max_ev, blend,
                             frames == 0 ? 1.0f : 0.0f};
        gpu.set_uniform(m_exposure_params, &params);
        gpu.set_texture(m_s_lum, 0, src);
        gpu.set_texture(m_s_prev, 1, prev->color_texture(0));
        draw_fullscreen(gpu, view, m_exposure_shader);

        res.publish(resource_names::EXPOSURE, out->color_texture(0));
        ++frames;
    }
} // namespace star::rendering
