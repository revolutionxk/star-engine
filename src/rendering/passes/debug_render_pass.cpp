#include "star/rendering/passes/debug_render_pass.hpp"

#include "star/graphics/device_context.hpp"
#include "star/rendering/debug_renderer.hpp"
#include "star/rendering/viewport.hpp"

namespace star::rendering {
    DebugRenderPass::DebugRenderPass(DebugRenderer& debug_renderer) : m_debug_renderer(debug_renderer) {}

    void DebugRenderPass::pre_render(const f32 /*delta_time*/) {
        m_debug_renderer.new_frame();
    }

    void DebugRenderPass::render(graphics::DeviceContext& context, const u32 view_id) {
        if (!m_viewport || !m_viewport->has_camera())
            return;

        m_viewport->bind_overlay(context, view_id);

        m_debug_renderer.flush(context, view_id, m_viewport->view_matrix(), m_viewport->projection_matrix());

        Super::render(context, view_id);
    }

    void DebugRenderPass::post_render(const f32 /*delta_time*/) {}

    u32 DebugRenderPass::reset(const u32 width, const u32 height) {
        if (m_viewport && (width > 0 || height > 0))
            m_viewport->resize(width, height);
        return Super::reset(width, height);
    }

} // namespace star::rendering
