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
        static void setup_dockspace();
        void render_main_menu_bar() const;
        void initialize_panels();

        EditorWindow* m_editor_window = nullptr;
        std::unique_ptr<rendering::Viewport> m_viewport;
        PanelManager m_panel_manager;
        ComponentInspector m_component_inspector;
        EditorInputManager m_input_manager;
        ViewportCameraSystem m_camera_system;
        GizmoSystem m_gizmo_system;
    };
} // namespace star::editor
