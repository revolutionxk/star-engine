#pragma once

#include "../core/editor_input_manager.hpp"
#include "../core/gizmo_system.hpp"
#include "panel.hpp"
#include "star/rendering/viewport.hpp"

namespace star::rendering {
    class PickingPass;
} // namespace star::rendering

namespace star::editor {
    class EditorWindow;

    class ScenePanel final : public Panel {
      public:
        ScenePanel() : Panel("Scene") {}

        void set_viewport(rendering::Viewport* viewport) noexcept {
            m_viewport = viewport;
        }

        void set_input_manager(EditorInputManager* input_manager) noexcept {
            m_input_manager = input_manager;
        }

        void set_gizmo_system(GizmoSystem* gizmo) noexcept {
            m_gizmo = gizmo;
        }

        void set_editor_window(EditorWindow* editor_window) noexcept {
            m_editor_window = editor_window;
        }

        void set_picking_pass(rendering::PickingPass* picking_pass) noexcept {
            m_picking_pass = picking_pass;
        }

        [[nodiscard]] bool is_focused() const noexcept {
            return m_focused;
        }

        [[nodiscard]] bool is_hovered() const noexcept {
            return m_hovered;
        }

        void on_imgui_render() override;

      private:
        void update_focus_state(bool gizmo_consuming);
        void handle_viewport_resize() const;
        void render_scene_texture();
        void render_gizmo_toolbar() const;
        void handle_click_pick() const;
        static void render_placeholder();
        static void render_error_message();

        rendering::Viewport* m_viewport = nullptr;
        EditorInputManager* m_input_manager = nullptr;
        GizmoSystem* m_gizmo = nullptr;
        EditorWindow* m_editor_window = nullptr;
        rendering::PickingPass* m_picking_pass = nullptr;
        ImVec2 m_image_pos{};
        ImVec2 m_image_size{};
        bool m_focused = false;
        bool m_hovered = false;
    };
} // namespace star::editor
