#include "star/rendering/passes/ibl_pass.hpp"

#include <algorithm>
#include <cmath>

#include "star/core/common.hpp"
#include "star/graphics/device.hpp"
#include "star/graphics/pipeline_state.hpp"
#include "star/rendering/systems/render_system.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/shader.hpp"

namespace star::rendering {
    IblPass::IblPass(graphics::Device& device, resources::ResourceManager& resources,
                     systems::RenderSystem& render_system)
        : m_device(device), m_resources(resources), m_render_system(render_system) {}

    IblPass::~IblPass() = default;

    bool IblPass::ensure_ready(graphics::DeviceContext& gpu) {
        const auto load = [this](const char* name, const resources::BuiltinShader id,
                                 graphics::ResourceHandle<graphics::Shader>& out) {
            if (out.is_valid())
                return;
            if (const auto res = m_resources.register_builtin_shader(name, id); res.is_valid()) {
                if (const auto* shader = m_resources.get_shader(res))
                    out = shader->handle;
            }
        };

        load("__ibl_brdf_shader", resources::BuiltinShader::IblBrdf, m_brdf_shader);
        load("__ibl_irradiance_shader", resources::BuiltinShader::IblIrradiance, m_irradiance_shader);
        load("__ibl_prefilter_shader", resources::BuiltinShader::IblPrefilter, m_prefilter_shader);

        if (!m_brdf_shader.is_valid() || !m_irradiance_shader.is_valid() || !m_prefilter_shader.is_valid())
            return false;

        if (!m_uniforms_resolved) {
            m_post_params = gpu.uniform("u_postParams", graphics::UniformType::Vec4);
            m_bake_params = gpu.uniform("u_iblBake", graphics::UniformType::Vec4);
            m_s_src = gpu.uniform("s_src", graphics::UniformType::Sampler);
            m_uniforms_resolved = true;
        }
        return true;
    }

    void IblPass::draw(graphics::DeviceContext& gpu, const u32 view_id,
                       const graphics::ResourceHandle<graphics::Shader> shader) const {
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

    void IblPass::bake_brdf(const RenderContext& ctx) {
        if (!m_brdf.is_valid() &&
            !m_brdf.create(&m_device, BRDF_SIZE, BRDF_SIZE, graphics::TextureFormat::RGBA16F, false))
            return;

        auto& gpu = ctx.gpu;
        const Vector4 post{m_device.caps().origin_bottom_left ? 0.0f : 1.0f, 0.0f, 0.0f, 0.0f};

        const u32 view = ctx.frame.views.acquire("IblBrdf");
        gpu.set_view_framebuffer(view, m_brdf.framebuffer());
        gpu.set_view_rect(view, 0, 0, static_cast<u16>(BRDF_SIZE), static_cast<u16>(BRDF_SIZE));
        gpu.set_view_clear(view, graphics::ClearFlags::None, 0, 1.0f, 0);
        gpu.set_uniform(m_post_params, &post);
        draw(gpu, view, m_brdf_shader);

        m_brdf_ready = true;
    }

    bool IblPass::bake_environment(const RenderContext& ctx, const graphics::ResourceHandle<graphics::Texture> source,
                                   const u32 source_width, const u32 source_height) {
        if (!m_irradiance.is_valid() && !m_irradiance.create(&m_device, IRRADIANCE_WIDTH, IRRADIANCE_HEIGHT,
                                                             graphics::TextureFormat::RGBA16F, false))
            return false;
        if (!m_specular.is_valid() &&
            !m_specular.create(&m_device, SPECULAR_WIDTH, SPECULAR_STRIP_HEIGHT * SPECULAR_LEVELS,
                               graphics::TextureFormat::RGBA16F, false))
            return false;

        auto& gpu = ctx.gpu;
        const Vector4 post{m_device.caps().origin_bottom_left ? 0.0f : 1.0f, 0.0f, 0.0f, 0.0f};

        const f32 width = static_cast<f32>(std::max(source_width, 1u));
        const f32 height = static_cast<f32>(std::max(source_height, 1u));
        const f32 max_lod = std::log2(std::max(width, height));
        const f32 sa_texel = 4.0f * 3.14159265359f / (width * height);
        const f32 irradiance_lod = std::max(std::log2(width / 16.0f), 0.0f);

        {
            const u32 view = ctx.frame.views.acquire("IblIrradiance");
            gpu.set_view_framebuffer(view, m_irradiance.framebuffer());
            gpu.set_view_rect(view, 0, 0, static_cast<u16>(IRRADIANCE_WIDTH), static_cast<u16>(IRRADIANCE_HEIGHT));
            gpu.set_view_clear(view, graphics::ClearFlags::None, 0, 1.0f, 0);

            const Vector4 bake{irradiance_lod, 0.0f, 0.0f, 0.0f};
            gpu.set_uniform(m_post_params, &post);
            gpu.set_uniform(m_bake_params, &bake);
            gpu.set_texture(m_s_src, 0, source);
            draw(gpu, view, m_irradiance_shader);
        }

        for (u32 level = 0; level < SPECULAR_LEVELS; ++level) {
            const u32 view = ctx.frame.views.acquire("IblPrefilter");
            gpu.set_view_framebuffer(view, m_specular.framebuffer());
            gpu.set_view_rect(view, 0, static_cast<u16>(level * SPECULAR_STRIP_HEIGHT),
                              static_cast<u16>(SPECULAR_WIDTH), static_cast<u16>(SPECULAR_STRIP_HEIGHT));
            gpu.set_view_clear(view, graphics::ClearFlags::None, 0, 1.0f, 0);

            const f32 roughness = static_cast<f32>(level) / static_cast<f32>(SPECULAR_LEVELS - 1);
            const Vector4 bake{roughness, sa_texel, max_lod, 0.0f};
            gpu.set_uniform(m_post_params, &post);
            gpu.set_uniform(m_bake_params, &bake);
            gpu.set_texture(m_s_src, 0, source);
            draw(gpu, view, m_prefilter_shader);
        }

        return true;
    }

    void IblPass::release_environment() {
        m_irradiance.destroy();
        m_specular.destroy();
        m_baked_source = {};
        m_render_system.set_ibl({});
    }

    void IblPass::render(const RenderContext& ctx) {
        const auto& env = m_render_system.environment();
        if (env.map == m_baked_source)
            return;

        if (!env.map.is_valid()) {
            release_environment();
            return;
        }

        if (!ensure_ready(ctx.gpu))
            return;

        if (!m_brdf_ready)
            bake_brdf(ctx);

        if (!bake_environment(ctx, env.map, env.width, env.height))
            return;

        systems::RenderSystem::IblState state;
        state.irradiance = m_irradiance.color_texture(0);
        state.specular = m_specular.color_texture(0);
        state.brdf_lut = m_brdf.color_texture(0);
        state.specular_levels = static_cast<f32>(SPECULAR_LEVELS);
        state.specular_strip_pad = 0.5f / static_cast<f32>(SPECULAR_STRIP_HEIGHT);
        m_render_system.set_ibl(state);

        m_baked_source = env.map;

        STAR_LOG_INFO(LogCategory::Rendering, "IBL baked: {}x{} source, {} specular levels", env.width, env.height,
                      SPECULAR_LEVELS);
    }
} // namespace star::rendering
