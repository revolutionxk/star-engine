#include "star/rendering/passes/postprocess_pass.hpp"

#include <algorithm>

#include "star/graphics/device.hpp"
#include "star/graphics/device_context.hpp"
#include "star/graphics/pipeline_state.hpp"
#include "star/math/math.hpp"
#include "star/rendering/viewport.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/shader.hpp"

namespace star::rendering {
    namespace {
        constexpr u32 POST_VIEW_BASE = 200;
        constexpr u32 POST_VIEWS_PER_VP = 11;
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
        load("__ssao_shader", resources::BuiltinShader::Ssao, m_ssao_shader);
        load("__ssao_blur_shader", resources::BuiltinShader::SsaoBlur, m_ssao_blur_shader);
        load("__ssr_shader", resources::BuiltinShader::Ssr, m_ssr_shader);
        load("__taa_shader", resources::BuiltinShader::Taa, m_taa_shader);
    }

    void PostProcessPass::resolve_uniforms(graphics::DeviceContext& gpu) {
        using graphics::UniformType;

        const auto vec4 = [&](const std::string_view name) { return gpu.uniform(name, UniformType::Vec4); };
        const auto mat4 = [&](const std::string_view name) { return gpu.uniform(name, UniformType::Mat4); };
        const auto sampler = [&](const std::string_view name) { return gpu.uniform(name, UniformType::Sampler); };

        m_ids.bloom_params = vec4("u_bloomParams");
        m_ids.blur_params = vec4("u_blurParams");
        m_ids.fxaa_params = vec4("u_fxaaParams");
        m_ids.ssao_inv_proj = mat4("u_ssaoInvProj");
        m_ids.ssao_proj = mat4("u_ssaoProj");
        m_ids.ssao_params = vec4("u_ssaoParams");
        m_ids.ssao_texel = vec4("u_ssaoTexel");
        m_ids.ssao_blur_params = vec4("u_ssaoBlurParams");
        m_ids.ssr_proj = mat4("u_ssrProj");
        m_ids.ssr_inv_proj = mat4("u_ssrInvProj");
        m_ids.ssr_params = vec4("u_ssrParams");
        m_ids.ssr_texel = vec4("u_ssrTexel");
        m_ids.taa_cur_inv_vp = mat4("u_taaCurInvVP");
        m_ids.taa_prev_vp = mat4("u_taaPrevVP");
        m_ids.taa_params = vec4("u_taaParams");
        m_ids.taa_texel = vec4("u_taaTexel");

        m_ids.s_src = sampler("s_src");
        m_ids.s_hdr = sampler("s_hdr");
        m_ids.s_bloom = sampler("s_bloom");
        m_ids.s_ao = sampler("s_ao");
        m_ids.s_depth = sampler("s_depth");
        m_ids.s_gbuffer = sampler("s_gbuffer");
        m_ids.s_scene = sampler("s_scene");
        m_ids.s_current = sampler("s_current");
        m_ids.s_history = sampler("s_history");
        m_ids.s_velocity = sampler("s_velocity");

        m_ids.post_params = vec4("u_postParams");
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

        if (!m_ids.is_resolved())
            resolve_uniforms(ctx.gpu);

        viewport->set_taa_enabled(m_settings.taa_enabled);

        if (ctx.frame.frame_index != m_frame) {
            m_frame = ctx.frame.frame_index;
            m_view_cursor = POST_VIEW_BASE;
        }
        const u32 base = m_view_cursor;
        m_view_cursor += POST_VIEWS_PER_VP;

        auto& gpu = ctx.gpu;
        auto& res = viewport->resources();
        const f32 flip_v = m_device.caps().origin_bottom_left ? 0.0f : 1.0f;
        const Vector4 flip_only{flip_v, 0.0f, 0.0f, 0.0f};
        const Vector4 no_flip{0.0f, 0.0f, 0.0f, 0.0f};

        graphics::ResourceHandle<graphics::Texture> ao_tex;
        const auto depth = viewport->hdr_depth_texture();
        if (m_settings.ssao_enabled && m_ssao_shader.is_valid() && m_ssao_blur_shader.is_valid() && depth.is_valid()) {
            const u32 aw = std::max(1u, viewport->width());
            const u32 ah = std::max(1u, viewport->height());
            RenderTarget* ssao_a = res.target("ssao_a", aw, ah, graphics::TextureFormat::RGBA8);
            RenderTarget* ssao_b = res.target("ssao_b", aw, ah, graphics::TextureFormat::RGBA8);
            if (ssao_a && ssao_b) {
                const auto sw = static_cast<u16>(aw);
                const auto sh = static_cast<u16>(ah);
                const Matrix4 proj = viewport->projection_matrix();
                const Matrix4 inv_proj = Matrix4::inverse(proj);

                gpu.set_view_framebuffer(base + 0, ssao_a->framebuffer());
                gpu.set_view_rect(base + 0, 0, 0, sw, sh);
                gpu.set_view_clear(base + 0, 0, 0, 1.0f, 0);
                gpu.set_uniform(m_ids.post_params, &no_flip);
                gpu.set_uniform(m_ids.ssao_inv_proj, &inv_proj);
                gpu.set_uniform(m_ids.ssao_proj, &proj);
                const Vector4 ssao_params{m_settings.ssao_radius, m_settings.ssao_power, m_settings.ssao_fade, 0.0f};
                gpu.set_uniform(m_ids.ssao_params, &ssao_params);
                const Vector4 ssao_texel{1.0f / static_cast<f32>(aw), 1.0f / static_cast<f32>(ah),
                                         flip_v, 0.0f};
                gpu.set_uniform(m_ids.ssao_texel, &ssao_texel);
                gpu.set_texture(m_ids.s_depth, 0, depth);
                gpu.set_texture(m_ids.s_gbuffer, 1, viewport->hdr_normal_texture());
                draw_fullscreen(gpu, base + 0, m_ssao_shader);

                gpu.set_view_framebuffer(base + 1, ssao_b->framebuffer());
                gpu.set_view_rect(base + 1, 0, 0, sw, sh);
                gpu.set_view_clear(base + 1, 0, 0, 1.0f, 0);
                gpu.set_uniform(m_ids.post_params, &no_flip);
                const Vector4 ao_blur{1.0f / static_cast<f32>(aw), 1.0f / static_cast<f32>(ah), 5000.0f,
                                      0.0f};
                gpu.set_uniform(m_ids.ssao_blur_params, &ao_blur);
                gpu.set_texture(m_ids.s_ao, 0, ssao_a->color_texture(0));
                gpu.set_texture(m_ids.s_depth, 1, depth);
                draw_fullscreen(gpu, base + 1, m_ssao_blur_shader);

                ao_tex = ssao_b->color_texture(0);
            }
        }

        graphics::ResourceHandle<graphics::Texture> lit = hdr;
        if (m_settings.ssr_enabled && m_ssr_shader.is_valid() && depth.is_valid()) {
            const u32 rw = std::max(1u, viewport->width());
            const u32 rh = std::max(1u, viewport->height());
            RenderTarget* ssr_a = res.target("ssr_a", rw, rh, graphics::TextureFormat::RGBA16F);
            RenderTarget* ssr_b = res.target("ssr_b", rw, rh, graphics::TextureFormat::RGBA16F);
            if (ssr_a && ssr_b) {
                const auto sw = static_cast<u16>(rw);
                const auto sh = static_cast<u16>(rh);
                const Matrix4 proj = viewport->projection_matrix();
                const Matrix4 inv_proj = Matrix4::inverse(proj);
                gpu.set_uniform(m_ids.ssr_proj, &proj);
                gpu.set_uniform(m_ids.ssr_inv_proj, &inv_proj);
                const Vector4 ssr_params{m_settings.ssr_max_distance, m_settings.ssr_thickness, 64.0f,
                                         m_settings.ssr_intensity};
                gpu.set_uniform(m_ids.ssr_params, &ssr_params);

                gpu.set_view_framebuffer(base + 2, ssr_a->framebuffer());
                gpu.set_view_rect(base + 2, 0, 0, sw, sh);
                gpu.set_view_clear(base + 2, 0, 0, 1.0f, 0);
                gpu.set_uniform(m_ids.post_params, &flip_only);
                const Vector4 ssr_march{1.0f / static_cast<f32>(rw), 1.0f / static_cast<f32>(rh), flip_v,
                                        0.0f};
                gpu.set_uniform(m_ids.ssr_texel, &ssr_march);
                gpu.set_texture(m_ids.s_hdr, 0, hdr);
                gpu.set_texture(m_ids.s_depth, 1, depth);
                gpu.set_texture(m_ids.s_gbuffer, 3, viewport->hdr_normal_texture());
                draw_fullscreen(gpu, base + 2, m_ssr_shader);

                gpu.set_view_framebuffer(base + 3, ssr_b->framebuffer());
                gpu.set_view_rect(base + 3, 0, 0, sw, sh);
                gpu.set_view_clear(base + 3, 0, 0, 1.0f, 0);
                gpu.set_uniform(m_ids.post_params, &flip_only);
                const Vector4 ssr_resolve{1.0f / static_cast<f32>(rw), 1.0f / static_cast<f32>(rh),
                                          flip_v, 1.0f};
                gpu.set_uniform(m_ids.ssr_texel, &ssr_resolve);
                gpu.set_texture(m_ids.s_hdr, 0, ssr_a->color_texture(0));
                gpu.set_texture(m_ids.s_depth, 1, depth);
                gpu.set_texture(m_ids.s_scene, 2, hdr);
                gpu.set_texture(m_ids.s_gbuffer, 3, viewport->hdr_normal_texture());
                draw_fullscreen(gpu, base + 3, m_ssr_shader);

                lit = ssr_b->color_texture(0);
            }
        }

        if (m_settings.taa_enabled && m_taa_shader.is_valid() && depth.is_valid()) {
            const u32 taw = std::max(1u, viewport->width());
            const u32 tah = std::max(1u, viewport->height());
            RenderTarget* taa_a = res.target("taa_a", taw, tah, graphics::TextureFormat::RGBA16F);
            RenderTarget* taa_b = res.target("taa_b", taw, tah, graphics::TextureFormat::RGBA16F);
            if (taa_a && taa_b) {
                u64& taa_frames = res.counter("taa_frames");
                const auto tw = static_cast<u16>(taw);
                const auto th = static_cast<u16>(tah);
                const bool even = (taa_frames % 2) == 0;
                RenderTarget* out = even ? taa_a : taa_b;
                RenderTarget* history = even ? taa_b : taa_a;

                const Matrix4 cur_inv_vp = Matrix4::inverse(viewport->cur_view_proj());
                const Matrix4 prev_vp = viewport->prev_view_proj();

                gpu.set_view_framebuffer(base + 4, out->framebuffer());
                gpu.set_view_rect(base + 4, 0, 0, tw, th);
                gpu.set_view_clear(base + 4, 0, 0, 1.0f, 0);
                gpu.set_uniform(m_ids.post_params, &flip_only);
                gpu.set_uniform(m_ids.taa_cur_inv_vp, &cur_inv_vp);
                gpu.set_uniform(m_ids.taa_prev_vp, &prev_vp);
                const Vector4 taa_params{flip_v, m_settings.taa_blend, taa_frames > 0 ? 1.0f : 0.0f, 0.0f};
                gpu.set_uniform(m_ids.taa_params, &taa_params);
                const Vector4 taa_texel{1.0f / static_cast<f32>(taw), 1.0f / static_cast<f32>(tah), 0.0f,
                                        0.0f};
                gpu.set_uniform(m_ids.taa_texel, &taa_texel);
                gpu.set_texture(m_ids.s_current, 0, lit);
                gpu.set_texture(m_ids.s_history, 1, history->color_texture(0));
                gpu.set_texture(m_ids.s_depth, 2, depth);
                gpu.set_texture(m_ids.s_velocity, 3, viewport->hdr_velocity_texture());
                draw_fullscreen(gpu, base + 4, m_taa_shader);

                lit = out->color_texture(0);
                ++taa_frames;
            }
        }

        graphics::ResourceHandle<graphics::Texture> bloom_tex;
        if (m_settings.bloom_enabled && m_bright_shader.is_valid() && m_blur_shader.is_valid()) {
            const u32 blw = std::max(1u, viewport->width() / 2);
            const u32 blh = std::max(1u, viewport->height() / 2);
            RenderTarget* bloom_a = res.target("bloom_a", blw, blh, graphics::TextureFormat::RGBA16F);
            RenderTarget* bloom_b = res.target("bloom_b", blw, blh, graphics::TextureFormat::RGBA16F);
            if (bloom_a && bloom_b) {
                const auto bw = static_cast<u16>(blw);
                const auto bh = static_cast<u16>(blh);

                gpu.set_view_framebuffer(base + 5, bloom_a->framebuffer());
                gpu.set_view_rect(base + 5, 0, 0, bw, bh);
                gpu.set_view_clear(base + 5, 0, 0, 1.0f, 0);
                gpu.set_uniform(m_ids.post_params, &flip_only);
                const Vector4 bloom_params{m_settings.bloom_threshold, m_settings.bloom_knee, 0.0f, 0.0f};
                gpu.set_uniform(m_ids.bloom_params, &bloom_params);
                gpu.set_texture(m_ids.s_hdr, 0, lit);
                draw_fullscreen(gpu, base + 5, m_bright_shader);

                gpu.set_view_framebuffer(base + 6, bloom_b->framebuffer());
                gpu.set_view_rect(base + 6, 0, 0, bw, bh);
                gpu.set_view_clear(base + 6, 0, 0, 1.0f, 0);
                gpu.set_uniform(m_ids.post_params, &flip_only);
                const Vector4 blur_h{1.0f / static_cast<f32>(blw), 0.0f, 0.0f, 0.0f};
                gpu.set_uniform(m_ids.blur_params, &blur_h);
                gpu.set_texture(m_ids.s_src, 0, bloom_a->color_texture(0));
                draw_fullscreen(gpu, base + 6, m_blur_shader);

                gpu.set_view_framebuffer(base + 7, bloom_a->framebuffer());
                gpu.set_view_rect(base + 7, 0, 0, bw, bh);
                gpu.set_view_clear(base + 7, 0, 0, 1.0f, 0);
                gpu.set_uniform(m_ids.post_params, &flip_only);
                const Vector4 blur_v{0.0f, 1.0f / static_cast<f32>(blh), 0.0f, 0.0f};
                gpu.set_uniform(m_ids.blur_params, &blur_v);
                gpu.set_texture(m_ids.s_src, 0, bloom_b->color_texture(0));
                draw_fullscreen(gpu, base + 7, m_blur_shader);

                bloom_tex = bloom_a->color_texture(0);
            }
        }

        const auto vw = static_cast<u16>(viewport->width());
        const auto vh = static_cast<u16>(viewport->height());

        const bool want_fxaa = m_settings.fxaa_enabled && !m_settings.taa_enabled && m_fxaa_shader.is_valid();
        RenderTarget* resolve = want_fxaa
                                    ? res.target("ldr_resolve", viewport->width(), viewport->height(),
                                                 graphics::TextureFormat::RGBA8)
                                    : nullptr;
        const bool fxaa = resolve != nullptr;

        const auto tonemap_fb = fxaa ? resolve->framebuffer() : display_fb;
        const u32 tonemap_view = base + 8;
        gpu.set_view_framebuffer(tonemap_view, tonemap_fb);
        gpu.set_view_rect(tonemap_view, 0, 0, vw, vh);
        gpu.set_view_clear(tonemap_view, 0, 0, 1.0f, 0);

        const f32 intensity = bloom_tex.is_valid() ? m_settings.bloom_intensity : 0.0f;
        const f32 ao_strength = ao_tex.is_valid() ? m_settings.ssao_strength : 0.0f;
        const Vector4 post_params{flip_v, m_settings.exposure, intensity, ao_strength};
        gpu.set_uniform(m_ids.post_params, &post_params);
        gpu.set_texture(m_ids.s_hdr, 0, lit);
        gpu.set_texture(m_ids.s_bloom, 1, bloom_tex.is_valid() ? bloom_tex : lit);
        gpu.set_texture(m_ids.s_ao, 2, ao_tex.is_valid() ? ao_tex : lit);
        draw_fullscreen(gpu, tonemap_view, m_tonemap_shader);

        if (fxaa) {
            const u32 fxaa_view = base + 9;
            gpu.set_view_framebuffer(fxaa_view, display_fb);
            gpu.set_view_rect(fxaa_view, 0, 0, vw, vh);
            gpu.set_view_clear(fxaa_view, 0, 0, 1.0f, 0);
            const Vector4 flip_params{flip_v, 0.0f, 0.0f, 0.0f};
            gpu.set_uniform(m_ids.post_params, &flip_params);
            const Vector4 fxaa_params{1.0f / static_cast<f32>(viewport->width()),
                                      1.0f / static_cast<f32>(viewport->height()), 0.0f, 0.0f};
            gpu.set_uniform(m_ids.fxaa_params, &fxaa_params);
            gpu.set_texture(m_ids.s_src, 0, resolve->color_texture(0));
            draw_fullscreen(gpu, fxaa_view, m_fxaa_shader);
        }
    }
} // namespace star::rendering
