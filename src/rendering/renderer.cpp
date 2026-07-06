#include "star/rendering/renderer.hpp"

#include "star/core/logger.hpp"
#include "star/ecs/components/camera.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/graphics/device.hpp"
#include "star/platform/window.hpp"
#include "star/rendering/debug_renderer.hpp"
#include "star/rendering/passes/debug_render_pass.hpp"
#include "star/rendering/passes/picking_pass.hpp"
#include "star/rendering/passes/postprocess_pass.hpp"
#include "star/rendering/passes/scene_render_pass.hpp"
#include "star/rendering/passes/shadow_pass.hpp"
#include "star/rendering/passes/sky_render_pass.hpp"
#include "star/rendering/render_scene.hpp"
#include "star/rendering/systems/render_system.hpp"
#include "star/rendering/viewport.hpp"

namespace star::rendering {
    Renderer::Renderer(graphics::Device& device, platform::Window& window, resources::ResourceManager& resource_manager)
        : m_device(&device), m_window(&window), m_resource_manager(&resource_manager) {
        if (m_initialized) {
            STAR_LOG_WARN(LogCategory::Rendering, "Renderer already initialized");
        }

        m_render_system = std::make_unique<systems::RenderSystem>(*m_device, *m_resource_manager);
        m_debug_renderer = std::make_unique<DebugRenderer>(*m_resource_manager);

        add_render_pass(std::make_unique<ShadowPass>(*m_device, *m_resource_manager, *m_render_system));
        add_render_pass(std::make_unique<SceneRenderPass>(*m_render_system));
        add_render_pass(std::make_unique<SkyRenderPass>(*m_render_system, *m_resource_manager));
        add_render_pass(std::make_unique<DebugRenderPass>(*m_debug_renderer));
        add_render_pass(std::make_unique<PostProcessPass>(*m_device, *m_resource_manager));
        add_render_pass(std::make_unique<PickingPass>(*m_device, *m_resource_manager));

        STAR_LOG_INFO(LogCategory::Rendering, "Window rendering setup complete");

        m_initialized = true;
        STAR_LOG_INFO(LogCategory::Rendering, "Renderer initialized successfully");
    }

    Renderer::~Renderer() {
        if (!m_initialized) {
            return;
        }

        STAR_LOG_INFO(LogCategory::Rendering, "Shutting down Renderer");

        const FrameContext empty{*m_device, *m_resource_manager, nullptr, nullptr, nullptr, 0.0f, m_frame_index};
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

        u32 view_id = 0;
        Viewport* primary = nullptr;

        if (!m_views.empty()) {
            for (const auto& [id, view] : m_views) {
                if (!view.enabled || !view.viewport)
                    continue;
                if (!primary)
                    primary = view.viewport;

                resolve_view_camera(view, frame.scene);

                FrameContext view_frame = frame;
                view_frame.viewport = view.viewport;
                view_frame.view = &view;
                view_id = m_graph.execute_scope(PassScope::PerView, view_frame, *context, view_id);
            }
        } else if (m_active_viewport) {
            const RenderView default_view{m_active_viewport, nullptr, nullptr, true, true};
            primary = m_active_viewport;

            resolve_view_camera(default_view, frame.scene);

            FrameContext view_frame = frame;
            view_frame.viewport = m_active_viewport;
            view_frame.view = &default_view;
            view_id = m_graph.execute_scope(PassScope::PerView, view_frame, *context, view_id);
        }

        FrameContext global_frame = frame;
        global_frame.viewport = primary ? primary : m_active_viewport;
        global_frame.view = nullptr;
        m_graph.execute_scope(PassScope::Global, global_frame, *context, view_id);

        context->end_frame();
    }

    void Renderer::resolve_view_camera(const RenderView& view, const RenderScene* scene) {
        if (view.camera && view.camera_transform) {
            view.viewport->set_camera(*view.camera, *view.camera_transform);
            return;
        }
        if (!scene || !scene->primary_camera.has_value())
            return;

        const auto& snapshot = *scene->primary_camera;
        components::Camera cam;
        cam.fov_y = snapshot.fov_y;
        cam.near_plane = snapshot.near_plane;
        cam.far_plane = snapshot.far_plane;
        cam.aspect_ratio = snapshot.aspect_ratio;

        components::Transform xf;
        xf.position = snapshot.position;
        xf.rotation = snapshot.rotation;
        xf.scale = snapshot.scale;

        view.viewport->set_camera(cam, xf);
    }

    ViewId Renderer::add_view(const RenderView& view) {
        const ViewId id = m_next_view_id++;
        m_views.push_back({id, view});
        return id;
    }

    void Renderer::remove_view(const ViewId id) {
        std::erase_if(m_views, [id](const ViewEntry& entry) { return entry.id == id; });
    }

    RenderView* Renderer::view(const ViewId id) {
        for (auto& entry : m_views) {
            if (entry.id == id)
                return &entry.view;
        }
        return nullptr;
    }

    void Renderer::clear_views() {
        m_views.clear();
    }

    void Renderer::post_render_passes(const FrameContext& frame) const {
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
            *m_device, *m_resource_manager, m_active_scene, m_active_viewport, nullptr, delta_time, m_frame_index,
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
