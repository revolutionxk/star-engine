#pragma once

#include <memory>

#include "editor_window.hpp"
#include "flecs.h"
#include "star/application/layer.hpp"

namespace star::rendering {
    class Viewport;
}

namespace star::editor {
    class EditorUILayer : public application::Layer {
      public:
        explicit EditorUILayer(EditorWindow* editor_window);
        ~EditorUILayer() override = default;

        bool initialize() override;
        void shutdown() override;
        void on_imgui_render() override;

      private:
        static void setup_dockspace();
        static void render_main_menu_bar();
        static void render_hierarchy_panel();
        void render_inspector_panel() const;
        static void render_metrics_panel();
        void render_scene_viewport_panel();
        static void render_console_panel();

      private:
        EditorWindow* m_editor_window;
        flecs::entity m_selected_entity;
        std::unique_ptr<rendering::Viewport> m_viewport;
        bool m_viewport_focused{false};
        bool m_viewport_hovered{false};
    };
} // namespace star::editor
