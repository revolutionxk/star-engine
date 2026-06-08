#include "star/rendering/renderer.hpp"

#include "star/core/logger.hpp"
#include "star/graphics/device.hpp"
#include "star/platform/window.hpp"
#include "star/rendering/debug_renderer.hpp"
#include "star/rendering/passes/debug_render_pass.hpp"
#include "star/rendering/passes/scene_render_pass.hpp"
#include "star/rendering/passes/sky_render_pass.hpp"
#include "star/rendering/render_scene.hpp"
#include "star/rendering/systems/render_system.hpp"

namespace star::rendering {
    Renderer::Renderer(graphics::Device& device, platform::Window& window, resources::ResourceManager& resource_manager)
        : m_device(&device), m_window(&window), m_resource_manager(&resource_manager) {
        if (m_initialized) {
            STAR_LOG_WARN(LogCategory::Rendering, "Renderer already initialized");
        }

        m_render_system = std::make_unique<systems::RenderSystem>(*m_device, *m_resource_manager);
        m_debug_renderer = std::make_unique<DebugRenderer>(*m_resource_manager);

        add_render_pass(std::make_unique<SceneRenderPass>(*m_render_system));
        add_render_pass(std::make_unique<SkyRenderPass>(*m_render_system, *m_resource_manager));
        add_render_pass(std::make_unique<DebugRenderPass>(*m_debug_renderer));

        STAR_LOG_INFO(LogCategory::Rendering, "Window rendering setup complete");

        m_initialized = true;
        STAR_LOG_INFO(LogCategory::Rendering, "Renderer initialized successfully");
    }

    Renderer::~Renderer() {
        if (!m_initialized) {
            return;
        }

        STAR_LOG_INFO(LogCategory::Rendering, "Shutting down Renderer");

        const FrameContext empty{*m_device, *m_resource_manager, nullptr, nullptr, 0.0f, m_frame_index};
        for (auto* pass : m_graph.ordered_view()) {
            pass->post_render(empty);
        }

        m_graph.clear();

        m_render_system.reset();
        m_debug_renderer.reset();
        m_initialized = false;
    }

    void Renderer::pre_render_passes(const FrameContext& frame) {
        m_graph.pre_render(frame);
    }

    void Renderer::submit_passes(const FrameContext& frame) {
        const auto context = m_device->context();
        context->begin_frame();
        m_graph.execute(frame, *context);
        context->end_frame();
    }

    void Renderer::post_render_passes(const FrameContext& frame) {
        m_graph.post_render(frame);
    }

    void Renderer::render_frame(const f32 delta_time) {
        if (!m_initialized) {
            return;
        }

        const FrameContext frame = make_frame_context(delta_time);

        pre_render_passes(frame);
        submit_passes(frame);
        post_render_passes(frame);
    }

    FrameContext Renderer::make_frame_context(const f32 delta_time) {
        ++m_frame_index;
        return FrameContext{
            *m_device,
            *m_resource_manager,
            m_active_scene,
            m_active_viewport,
            delta_time,
            m_frame_index,
        };
    }

    void Renderer::add_render_pass(std::unique_ptr<IRenderPass> render_pass) {
        m_graph.add_pass(std::move(render_pass));
    }

    void Renderer::remove_render_pass(const char* name) {
        m_graph.remove_pass(name);
    }

    void Renderer::reset_render_passes(const u32 width, const u32 height) {
        if (!m_initialized) {
            STAR_LOG_WARN(LogCategory::Rendering, "Cannot reset render passes - renderer not initialized");
            return;
        }

        STAR_LOG_INFO(LogCategory::Rendering, "Resetting render passes with size: {}x{}", width, height);

        m_graph.reset(width, height, *m_device->context());

        STAR_LOG_INFO(LogCategory::Rendering, "All render passes reset successfully");
    }
} // namespace star::rendering
