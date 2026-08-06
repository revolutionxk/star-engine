#include "star/rendering/procedural_sky.hpp"

#include "star/core/common.hpp"
#include "star/graphics/device_context.hpp"
#include "star/graphics/pipeline_state.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/builtin_shaders.hpp"
#include "star/resources/shader/shader.hpp"

namespace star::rendering {
    ProceduralSky::~ProceduralSky() {
        shutdown();
    }

    void ProceduralSky::initialize(resources::ResourceManager& rm) {
        if (m_initialized)
            return;

        m_rm = &rm;

        if (const auto slot = rm.register_builtin_shader("__sky_shader", resources::BuiltinShader::Atmosphere);
            slot.is_valid()) {
            if (const auto* s = rm.get_shader(slot))
                m_shader = s->handle;
            else
                STAR_LOG_ERROR(LogCategory::Rendering, "ProceduralSky: shader resource missing after creation");
        } else {
            STAR_LOG_ERROR(LogCategory::Rendering, "ProceduralSky: failed to create embedded sky shader");
            return;
        }

        build_grid();
        m_initialized = true;
        STAR_LOG_INFO(LogCategory::Rendering, "ProceduralSky initialized ({} verts, {} indices)", m_vertices.size() / 2,
                      m_indices.size());
    }

    void ProceduralSky::shutdown() {
        if (!m_initialized)
            return;

        m_vertices.clear();
        m_indices.clear();
        m_initialized = false;
    }

    void ProceduralSky::build_grid() {
        m_vertices.reserve(GRID * GRID * 2);
        for (int i = 0; i < GRID; ++i) {
            for (int j = 0; j < GRID; ++j) {
                m_vertices.push_back(static_cast<float>(j) / (GRID - 1) * 2.0f - 1.0f);
                m_vertices.push_back(static_cast<float>(i) / (GRID - 1) * 2.0f - 1.0f);
            }
        }

        m_indices.reserve((GRID - 1) * (GRID - 1) * 6);
        for (int i = 0; i < GRID - 1; ++i) {
            for (int j = 0; j < GRID - 1; ++j) {
                const auto tl = static_cast<u16>(j + 0 + GRID * (i + 0));
                const auto tr = static_cast<u16>(j + 1 + GRID * (i + 0));
                const auto bl = static_cast<u16>(j + 0 + GRID * (i + 1));
                const auto br = static_cast<u16>(j + 1 + GRID * (i + 1));

                m_indices.push_back(tl);
                m_indices.push_back(tr);
                m_indices.push_back(bl);
                m_indices.push_back(tr);
                m_indices.push_back(br);
                m_indices.push_back(bl);
            }
        }
    }

    void ProceduralSky::resolve_uniforms(graphics::DeviceContext& context) const {
        using graphics::UniformType;

        m_ids.sun_direction = context.uniform("u_sunDirection", UniformType::Vec4);
        m_ids.sky_luminance_xyz = context.uniform("u_skyLuminanceXYZ", UniformType::Vec4);
        m_ids.sun_luminance = context.uniform("u_sunLuminance", UniformType::Vec4);
        m_ids.parameters = context.uniform("u_parameters", UniformType::Vec4);
        m_ids.perez_coeff = context.uniform("u_perezCoeff", UniformType::Vec4, 5);
        m_ids.sampler_env = context.uniform("s_envMap", UniformType::Sampler);
        m_ids.env_sky_mode = context.uniform("u_envSkyMode", UniformType::Vec4);
    }

    void ProceduralSky::draw(graphics::DeviceContext& context, const u32 view_id,
                             const graphics::ResourceHandle<graphics::Texture> env_map, const f32 env_intensity) const {
        if (!m_initialized || !m_shader.is_valid())
            return;

        if (!m_ids.is_resolved())
            resolve_uniforms(context);

        const bool use_env = env_map.is_valid();
        const float env_mode[4] = {use_env ? 1.0f : 0.0f, env_intensity, 0.0f, 0.0f};
        context.set_uniform(m_ids.env_sky_mode, env_mode);
        if (use_env)
            context.set_texture(m_ids.sampler_env, 0, env_map);
        else if (m_rm)
            if (const auto* black = m_rm->get_texture(m_rm->black_texture()))
                context.set_texture(m_ids.sampler_env, 0, black->handle);

        const float sun_dir[4] = {m_params.sun_direction.x, m_params.sun_direction.y, m_params.sun_direction.z, 0.0f};
        context.set_uniform(m_ids.sun_direction, sun_dir);

        const float sky_xyz[4] = {m_params.sky_luminance_xyz.x, m_params.sky_luminance_xyz.y,
                                  m_params.sky_luminance_xyz.z, 0.0f};
        context.set_uniform(m_ids.sky_luminance_xyz, sky_xyz);

        const float sun_lum[4] = {m_params.sun_luminance.x, m_params.sun_luminance.y, m_params.sun_luminance.z, 0.0f};
        context.set_uniform(m_ids.sun_luminance, sun_lum);

        const float params[4] = {m_params.sun_size, m_params.sun_bloom, m_params.exposition, m_params.time};
        context.set_uniform(m_ids.parameters, params);

        context.set_uniform(m_ids.perez_coeff, m_params.perez_coeff, 5);

        constexpr graphics::PipelineState sky_state{
            .blend_mode = graphics::BlendMode::Opaque,
            .cull = graphics::CullMode::None,
            .depth_test = graphics::DepthTest::Equal,
            .depth_write = false,
        };
        context.set_pipeline_state(sky_state);

        context.set_transient_vertex_buffer(0, m_vertices.data(), static_cast<u32>(m_vertices.size() / 2),
                                            graphics::VertexLayoutType::ScreenPos);
        context.set_transient_index_buffer(m_indices.data(), static_cast<u32>(m_indices.size()));

        context.submit(view_id, m_shader);
    }
} // namespace star::rendering
