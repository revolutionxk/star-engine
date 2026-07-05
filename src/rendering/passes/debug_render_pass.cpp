#include "star/rendering/passes/debug_render_pass.hpp"

#include "star/graphics/device_context.hpp"
#include "star/rendering/debug_renderer.hpp"
#include "star/rendering/render_view.hpp"
#include "star/rendering/viewport.hpp"

namespace star::rendering {
    DebugRenderPass::DebugRenderPass(DebugRenderer& debug_renderer) : m_debug_renderer(debug_renderer) {}

    void DebugRenderPass::pre_render(const FrameContext& /*frame*/) {
        m_debug_renderer.new_frame();
    }

    void DebugRenderPass::render(const RenderContext& ctx) {
        if (!ctx.frame.view || !ctx.frame.view->draw_debug)
            return;

        const Viewport* viewport = ctx.frame.viewport;
        if (!viewport || !viewport->has_camera())
            return;

        viewport->bind_overlay(ctx.gpu, ctx.view_id);

        m_debug_renderer.flush(ctx.gpu, ctx.view_id, viewport->view_matrix(), viewport->projection_matrix());

        Super::render(ctx);
    }

    u32 DebugRenderPass::reset(const u32 width, const u32 height) {
        return Super::reset(width, height);
    }

} // namespace star::rendering
