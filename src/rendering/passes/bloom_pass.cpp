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
        : PostPass(device, resources, settings, "__bloom_down_shader", resources::BuiltinShader::BloomDown) {}

    void BloomPass::resolve_uniforms(graphics::DeviceContext& gpu) {
        using graphics::UniformType;

        m_bloom_params = gpu.uniform("u_bloomParams", UniformType::Vec4);
        m_bloom_texel = gpu.uniform("u_bloomTexel", UniformType::Vec4);
        m_s_src = gpu.uniform("s_src", UniformType::Sampler);
    }

    void BloomPass::render(const RenderContext& ctx) {
        Viewport* viewport = ctx.frame.viewport;
        if (!m_settings.enabled || !m_settings.bloom_enabled || !viewport)
            return;

        auto& gpu = ctx.gpu;
        if (!ensure_ready(gpu))
            return;

        if (!m_upsample_shader.is_valid()) {
            const auto res =
                m_resources.register_builtin_shader("__bloom_up_shader", resources::BuiltinShader::BloomUp);
            if (const auto* shader = m_resources.get_shader(res))
                m_upsample_shader = shader->handle;
        }
        if (!m_upsample_shader.is_valid())
            return;

        auto& res = viewport->resources();
        const auto lit = res.texture(resource_names::LIT);
        if (!lit.is_valid())
            return;

        std::array<u32, MAX_MIPS> widths{};
        std::array<u32, MAX_MIPS> heights{};
        std::array<RenderTarget*, MAX_MIPS> chain{};

        u32 w = std::max(1u, viewport->width() / 2);
        u32 h = std::max(1u, viewport->height() / 2);
        u32 mips = 0;

        char name[16];
        for (u32 i = 0; i < MAX_MIPS && w >= MIN_MIP_SIZE && h >= MIN_MIP_SIZE; ++i) {
            std::snprintf(name, sizeof(name), "bloom_%u", i);
            RenderTarget* target = res.target(name, w, h, graphics::TextureFormat::RGBA16F);
            if (!target)
                break;

            widths[i] = w;
            heights[i] = h;
            chain[i] = target;
            ++mips;

            w = std::max(1u, w / 2);
            h = std::max(1u, h / 2);
        }

        if (mips == 0)
            return;

        const Vector4 flip_only{flip_v(), 0.0f, 0.0f, 0.0f};

        for (u32 i = 0; i < mips; ++i) {
            const u32 view = ctx.frame.views.acquire("BloomDown");
            begin_view(gpu, view, *chain[i]);
            gpu.set_uniform(m_post_params, &flip_only);

            const bool first = i == 0;
            const u32 src_w = first ? viewport->width() : widths[i - 1];
            const u32 src_h = first ? viewport->height() : heights[i - 1];

            const Vector4 params{m_settings.bloom_threshold, m_settings.bloom_knee, first ? 1.0f : 0.0f, 0.0f};
            gpu.set_uniform(m_bloom_params, &params);

            const Vector4 texel{1.0f / static_cast<f32>(src_w), 1.0f / static_cast<f32>(src_h), 1.0f, 0.0f};
            gpu.set_uniform(m_bloom_texel, &texel);

            gpu.set_texture(m_s_src, 0, first ? lit : chain[i - 1]->color_texture(0));
            draw_fullscreen(gpu, view);
        }

        for (u32 i = mips - 1; i > 0; --i) {
            const u32 view = ctx.frame.views.acquire("BloomUp");
            begin_view(gpu, view, *chain[i - 1]);
            gpu.set_uniform(m_post_params, &flip_only);

            const Vector4 texel{1.0f / static_cast<f32>(widths[i]), 1.0f / static_cast<f32>(heights[i]),
                                m_settings.bloom_radius, 0.0f};
            gpu.set_uniform(m_bloom_texel, &texel);

            gpu.set_texture(m_s_src, 0, chain[i]->color_texture(0));
            draw_fullscreen(gpu, view, m_upsample_shader, graphics::BlendMode::Additive);
        }

        res.publish(resource_names::BLOOM, chain[0]->color_texture(0));
    }
} // namespace star::rendering
