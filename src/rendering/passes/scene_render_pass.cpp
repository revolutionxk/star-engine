#include "star/rendering/passes/scene_render_pass.hpp"

#include "star/graphics/device_context.hpp"
#include "star/rendering/render_scene.hpp"
#include "star/rendering/systems/render_system.hpp"
#include "star/rendering/viewport.hpp"

namespace star::rendering {

    SceneRenderPass::SceneRenderPass(systems::RenderSystem& render_system) : m_render_system(render_system) {}

    void SceneRenderPass::render(const RenderContext& ctx) {
        Viewport* viewport = ctx.frame.viewport;
        if (viewport) {
            viewport->bind(ctx.gpu, ctx.view_id);
        }

        const RenderScene* scene = ctx.frame.scene;
        if (!scene || !scene->active) {
            return;
        }

        m_render_system.render(*scene, ctx.gpu, ctx.view_id, viewport);

        Super::render(ctx);
    }


} // namespace star::rendering
