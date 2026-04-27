#pragma once

#include "../core/editor_input_manager.hpp"
#include "../core/gizmo_system.hpp"
#include "panel.hpp"
#include "star/rendering/viewport.hpp"

namespace star::editor {
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
        static void render_placeholder();
        static void render_error_message();

        rendering::Viewport* m_viewport = nullptr;
        EditorInputManager* m_input_manager = nullptr;
        GizmoSystem* m_gizmo = nullptr;
        ImVec2 m_image_pos{};
        ImVec2 m_image_size{};
        bool m_focused = false;
        bool m_hovered = false;
    };
} // namespace star::editor
