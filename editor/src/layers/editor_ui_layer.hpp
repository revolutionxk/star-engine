#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include <memory>

#include "../core/editor_input_manager.hpp"
#include "../core/gizmo_system.hpp"
#include "../core/panel_manager.hpp"
#include "../core/viewport_camera_system.hpp"
#include "../panels/component_inspector.hpp"
#include "editor_window.hpp"
#include "star/application/layer.hpp"
#include "star/ecs/components/camera.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/rendering/render_view.hpp"
#include "star/rendering/viewport.hpp"

namespace star::editor {
    class EditorUILayer final : public application::Layer {
      public:
        explicit EditorUILayer(EditorWindow* editor_window);
        ~EditorUILayer() override = default;

        EditorUILayer(const EditorUILayer&) = delete;
        EditorUILayer& operator=(const EditorUILayer&) = delete;
        EditorUILayer(EditorUILayer&&) = delete;
        EditorUILayer& operator=(EditorUILayer&&) = delete;

        bool initialize() override;
        void shutdown() override;
        void update(f32 dt) override;
        void pre_render(f32 dt) override;
        void render() override;
        void on_imgui_render() override;
        void on_imgui_init() override;

      private:
        void setup_dockspace();
        static void build_default_layout(unsigned dockspace_id);
        void render_main_menu_bar();
        void render_toolbar() const;
        void render_project_browser();
        void initialize_panels();

        bool m_rebuild_layout = false;

        bool m_open_browser_requested = false;
        char m_new_project_name[128]{};
        char m_new_project_location[512]{};
        char m_open_project_path[512]{};

        EditorWindow* m_editor_window = nullptr;

        std::unique_ptr<rendering::Viewport> m_scene_viewport;
        std::unique_ptr<rendering::Viewport> m_game_viewport;

        components::Transform m_editor_cam_xf;
        components::Camera m_editor_cam;
        rendering::ViewId m_scene_view = rendering::INVALID_VIEW;
        rendering::ViewId m_game_view = rendering::INVALID_VIEW;

        PanelManager m_panel_manager;
        ComponentInspector m_component_inspector;
        EditorInputManager m_input_manager;
        ViewportCameraSystem m_camera_system;
        GizmoSystem m_gizmo_system;
    };
} // namespace star::editor
