#include "star/rendering/passes/sky_render_pass.hpp"

#include "star/core/common.hpp"
#include "star/rendering/render_scene.hpp"
#include "star/rendering/sky/atmosphere_solver.hpp"
#include "star/rendering/systems/render_system.hpp"
#include "star/rendering/viewport.hpp"

namespace star::rendering {
    SkyRenderPass::SkyRenderPass(systems::RenderSystem& render_system, resources::ResourceManager& resource_manager)
        : m_render_system(render_system) {
        m_sky.initialize(resource_manager);
        STAR_LOG_INFO(LogCategory::Rendering, "SkyRenderPass initialized");
    }

    SkyRenderPass::~SkyRenderPass() = default;

    void SkyRenderPass::pre_render(const FrameContext& frame) {
        m_elapsed_time += frame.delta_time;
        m_has_sky_this_frame = false;

        const RenderScene* scene = frame.scene;
        if (!scene || !scene->active) {
            m_render_system.clear_atmospheric_lighting();
            return;
        }

        if (!scene->atmosphere.has_value() || !scene->atmosphere->enabled) {
            m_render_system.clear_atmospheric_lighting();
            return;
        }

        const auto [sky_params, lighting] = solve_atmosphere(*scene->atmosphere, m_elapsed_time);
        m_sky.set_params(sky_params);
        m_render_system.apply_atmospheric_lighting(lighting);
        m_has_sky_this_frame = true;
    }

    void SkyRenderPass::render(const RenderContext& ctx) {
        Super::render(ctx);

        const auto& env = m_render_system.environment();
        const bool use_env = env.map.is_valid();
        if (!m_has_sky_this_frame && !use_env) {
            return;
        }

        if (const auto* viewport = ctx.frame.viewport) {
            viewport->bind_overlay(ctx.gpu, ctx.view_id);
        }

        m_sky.draw(ctx.gpu, ctx.view_id, use_env ? env.map : graphics::ResourceHandle<graphics::Texture>{},
                   env.intensity);
    }
} // namespace star::rendering
