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

    void ProceduralSky::draw(graphics::DeviceContext& context, const u32 view_id) const {
        if (!m_initialized || !m_shader.is_valid())
            return;

        const float sun_dir[4] = {m_params.sun_direction.x, m_params.sun_direction.y, m_params.sun_direction.z, 0.0f};
        context.set_uniform("u_sunDirection", sun_dir, 1, graphics::UniformType::Vec4);

        const float sky_xyz[4] = {m_params.sky_luminance_xyz.x, m_params.sky_luminance_xyz.y,
                                  m_params.sky_luminance_xyz.z, 0.0f};
        context.set_uniform("u_skyLuminanceXYZ", sky_xyz, 1, graphics::UniformType::Vec4);

        const float sun_lum[4] = {m_params.sun_luminance.x, m_params.sun_luminance.y, m_params.sun_luminance.z, 0.0f};
        context.set_uniform("u_sunLuminance", sun_lum, 1, graphics::UniformType::Vec4);

        const float params[4] = {m_params.sun_size, m_params.sun_bloom, m_params.exposition, m_params.time};
        context.set_uniform("u_parameters", params, 1, graphics::UniformType::Vec4);

        context.set_uniform("u_perezCoeff", m_params.perez_coeff, 5, graphics::UniformType::Vec4);

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
