#include "star/rendering/passes/postprocess_pass.hpp"

#include <algorithm>

#include "star/graphics/device.hpp"
#include "star/graphics/device_context.hpp"
#include "star/graphics/pipeline_state.hpp"
#include "star/rendering/viewport.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/shader.hpp"

namespace star::rendering {
    namespace {
        constexpr u32 POST_VIEW_BASE = 200;
        constexpr u32 POST_VIEWS_PER_VP = 8;
    } // namespace

    PostProcessPass::PostProcessPass(graphics::Device& device, resources::ResourceManager& resources)
        : m_device(device), m_resources(resources) {}

    PostProcessPass::~PostProcessPass() = default;

    void PostProcessPass::ensure_shaders() {
        const auto load = [this](const char* name, const resources::BuiltinShader id,
                                 graphics::ResourceHandle<graphics::Shader>& out) {
            if (out.is_valid())
                return;
            if (const auto res = m_resources.register_builtin_shader(name, id); res.is_valid())
                if (const auto* shader = m_resources.get_shader(res))
                    out = shader->handle;
        };
        load("__tonemap_shader", resources::BuiltinShader::Tonemap, m_tonemap_shader);
        load("__bloom_bright_shader", resources::BuiltinShader::BloomBright, m_bright_shader);
        load("__bloom_blur_shader", resources::BuiltinShader::BloomBlur, m_blur_shader);
        load("__fxaa_shader", resources::BuiltinShader::Fxaa, m_fxaa_shader);
    }

    void PostProcessPass::draw_fullscreen(graphics::DeviceContext& gpu, const u32 view_id,
                                          const graphics::ResourceHandle<graphics::Shader> shader) {
        static constexpr f32 k_tri[6] = {-1.0f, -1.0f, 3.0f, -1.0f, -1.0f, 3.0f};
        constexpr graphics::PipelineState state{
            .blend_mode = graphics::BlendMode::Opaque,
            .cull = graphics::CullMode::None,
            .depth_test = graphics::DepthTest::None,
            .depth_write = false,
        };
        gpu.set_pipeline_state(state);
        gpu.set_transient_vertex_buffer(0, k_tri, 3, graphics::VertexLayoutType::ScreenPos);
        gpu.submit(view_id, shader);
    }

    PostProcessPass::BloomTargets* PostProcessPass::bloom_targets_for(const Viewport& viewport) {
        const u32 bw = std::max(1u, viewport.width() / 2);
        const u32 bh = std::max(1u, viewport.height() / 2);

        auto& bt = m_bloom[&viewport];
        if (bt.width == bw && bt.height == bh && bt.a && bt.b && bt.a->is_valid() && bt.b->is_valid())
            return &bt;

        bt.a = std::make_unique<RenderTarget>();
        bt.b = std::make_unique<RenderTarget>();
        const bool ok_a = bt.a->create(&m_device, bw, bh, graphics::TextureFormat::RGBA16F, false);
        const bool ok_b = bt.b->create(&m_device, bw, bh, graphics::TextureFormat::RGBA16F, false);
        if (!ok_a || !ok_b) {
            bt.a.reset();
            bt.b.reset();
            bt.width = bt.height = 0;
            return nullptr;
        }
        bt.width = bw;
        bt.height = bh;
        return &bt;
    }

    PostProcessPass::ResolveTarget* PostProcessPass::resolve_target_for(const Viewport& viewport) {
        const u32 w = std::max(1u, viewport.width());
        const u32 h = std::max(1u, viewport.height());

        auto& rt = m_resolve[&viewport];
        if (rt.width == w && rt.height == h && rt.ldr && rt.ldr->is_valid())
            return &rt;

        rt.ldr = std::make_unique<RenderTarget>();
        if (!rt.ldr->create(&m_device, w, h, graphics::TextureFormat::RGBA8, false)) {
            rt.ldr.reset();
            rt.width = rt.height = 0;
            return nullptr;
        }
        rt.width = w;
        rt.height = h;
        return &rt;
    }

    void PostProcessPass::render(const RenderContext& ctx) {
        Viewport* viewport = ctx.frame.viewport;
        if (!m_settings.enabled || !viewport)
            return;

        const auto hdr = viewport->hdr_color_texture();
        const auto display_fb = viewport->display_framebuffer();
        if (!hdr.is_valid() || !display_fb.is_valid())
            return;

        ensure_shaders();
        if (!m_tonemap_shader.is_valid())
            return;

        if (ctx.frame.frame_index != m_frame) {
            m_frame = ctx.frame.frame_index;
            m_view_cursor = POST_VIEW_BASE;
        }
        const u32 base = m_view_cursor;
        m_view_cursor += POST_VIEWS_PER_VP;

        auto& gpu = ctx.gpu;
        const f32 flip_v = m_device.caps().origin_bottom_left ? 0.0f : 1.0f;

        graphics::ResourceHandle<graphics::Texture> bloom_tex;
        if (m_settings.bloom_enabled && m_bright_shader.is_valid() && m_blur_shader.is_valid()) {
            if (const BloomTargets* bt = bloom_targets_for(*viewport)) {
                const auto bw = static_cast<u16>(bt->width);
                const auto bh = static_cast<u16>(bt->height);
                const Vector4 flip_only{flip_v, 0.0f, 0.0f, 0.0f};

                gpu.set_view_framebuffer(base + 0, bt->a->framebuffer());
                gpu.set_view_rect(base + 0, 0, 0, bw, bh);
                gpu.set_view_clear(base + 0, 0, 0, 1.0f, 0);
                gpu.set_uniform("u_postParams", &flip_only, 1, graphics::UniformType::Vec4);
                const Vector4 bloom_params{m_settings.bloom_threshold, m_settings.bloom_knee, 0.0f, 0.0f};
                gpu.set_uniform("u_bloomParams", &bloom_params, 1, graphics::UniformType::Vec4);
                gpu.set_texture(0, hdr);
                draw_fullscreen(gpu, base + 0, m_bright_shader);

                gpu.set_view_framebuffer(base + 1, bt->b->framebuffer());
                gpu.set_view_rect(base + 1, 0, 0, bw, bh);
                gpu.set_view_clear(base + 1, 0, 0, 1.0f, 0);
                gpu.set_uniform("u_postParams", &flip_only, 1, graphics::UniformType::Vec4);
                const Vector4 blur_h{1.0f / static_cast<f32>(bt->width), 0.0f, 0.0f, 0.0f};
                gpu.set_uniform("u_blurParams", &blur_h, 1, graphics::UniformType::Vec4);
                gpu.set_texture(0, bt->a->color_texture(0));
                draw_fullscreen(gpu, base + 1, m_blur_shader);

                gpu.set_view_framebuffer(base + 2, bt->a->framebuffer());
                gpu.set_view_rect(base + 2, 0, 0, bw, bh);
                gpu.set_view_clear(base + 2, 0, 0, 1.0f, 0);
                gpu.set_uniform("u_postParams", &flip_only, 1, graphics::UniformType::Vec4);
                const Vector4 blur_v{0.0f, 1.0f / static_cast<f32>(bt->height), 0.0f, 0.0f};
                gpu.set_uniform("u_blurParams", &blur_v, 1, graphics::UniformType::Vec4);
                gpu.set_texture(0, bt->b->color_texture(0));
                draw_fullscreen(gpu, base + 2, m_blur_shader);

                bloom_tex = bt->a->color_texture(0);
            }
        }

        const auto vw = static_cast<u16>(viewport->width());
        const auto vh = static_cast<u16>(viewport->height());

        const bool want_fxaa = m_settings.fxaa_enabled && m_fxaa_shader.is_valid();
        ResolveTarget* rt = want_fxaa ? resolve_target_for(*viewport) : nullptr;
        const bool fxaa = rt != nullptr;

        const auto tonemap_fb = fxaa ? rt->ldr->framebuffer() : display_fb;
        const u32 tonemap_view = base + 3;
        gpu.set_view_framebuffer(tonemap_view, tonemap_fb);
        gpu.set_view_rect(tonemap_view, 0, 0, vw, vh);
        gpu.set_view_clear(tonemap_view, 0, 0, 1.0f, 0);

        const f32 intensity = bloom_tex.is_valid() ? m_settings.bloom_intensity : 0.0f;
        const Vector4 post_params{flip_v, m_settings.exposure, intensity, 0.0f};
        gpu.set_uniform("u_postParams", &post_params, 1, graphics::UniformType::Vec4);
        gpu.set_texture(0, hdr);
        gpu.set_texture(1, bloom_tex.is_valid() ? bloom_tex : hdr);
        draw_fullscreen(gpu, tonemap_view, m_tonemap_shader);

        if (fxaa) {
            const u32 fxaa_view = base + 4;
            gpu.set_view_framebuffer(fxaa_view, display_fb);
            gpu.set_view_rect(fxaa_view, 0, 0, vw, vh);
            gpu.set_view_clear(fxaa_view, 0, 0, 1.0f, 0);
            const Vector4 flip_params{flip_v, 0.0f, 0.0f, 0.0f};
            gpu.set_uniform("u_postParams", &flip_params, 1, graphics::UniformType::Vec4);
            const Vector4 fxaa_params{1.0f / static_cast<f32>(viewport->width()),
                                      1.0f / static_cast<f32>(viewport->height()), 0.0f, 0.0f};
            gpu.set_uniform("u_fxaaParams", &fxaa_params, 1, graphics::UniformType::Vec4);
            gpu.set_texture(0, rt->ldr->color_texture(0));
            draw_fullscreen(gpu, fxaa_view, m_fxaa_shader);
        }
    }
} // namespace star::rendering
