#include "star/rendering/passes/scene_render_pass.hpp"

#include "star/graphics/device_context.hpp"
#include "star/scene/scene.hpp"
#include "star/systems/render_system.hpp"

namespace star::rendering {

    SceneRenderPass::SceneRenderPass(systems::RenderSystem& render_system) : m_render_system(render_system) {}

    void SceneRenderPass::pre_render(const f32 delta_time) {}

    void SceneRenderPass::render(graphics::DeviceContext& context, const u32 view_id) {
        if (!m_scene || !m_scene->is_active()) {
            return;
        }

        m_render_system.render(*m_scene, context, view_id);
    }

    void SceneRenderPass::post_render(f32 delta_time) {}

    void SceneRenderPass::reset(const u32 width, const u32 height) {
        m_render_system.set_viewport_size(width, height);
    }

} // namespace star::rendering
