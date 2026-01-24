#pragma once

#include <imgui.h>

#include "panel.hpp"
#include "star/rendering/viewport.hpp"

namespace star::editor {

    class ScenePanel final : public Panel {
      public:
        ScenePanel() : Panel("Scene") {}

        void set_viewport(rendering::Viewport* viewport) noexcept {
            m_viewport = viewport;
        }

        [[nodiscard]] bool is_focused() const noexcept {
            return m_focused;
        }

        [[nodiscard]] bool is_hovered() const noexcept {
            return m_hovered;
        }

        void on_imgui_render() override {
            if (!m_is_open)
                return;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::Begin(m_name.c_str(), &m_is_open);

            update_focus_state();
            handle_viewport_resize();
            render_scene_texture();
            render_gizmo_toolbar();

            ImGui::End();
            ImGui::PopStyleVar();
        }

      private:
        void update_focus_state() {
            m_focused = ImGui::IsWindowFocused();
            m_hovered = ImGui::IsWindowHovered();
        }

        void handle_viewport_resize() {
            if (!m_viewport)
                return;

            const ImVec2 viewport_size = ImGui::GetContentRegionAvail();
            if (viewport_size.x <= 0 || viewport_size.y <= 0)
                return;

            const u32 new_width = static_cast<u32>(viewport_size.x);
            const u32 new_height = static_cast<u32>(viewport_size.y);

            if (needs_resize(new_width, new_height)) {
                m_viewport->resize(new_width, new_height);
            }
        }

        [[nodiscard]] bool needs_resize(u32 width, u32 height) const noexcept {
            return m_viewport && (width != m_viewport->width() || height != m_viewport->height());
        }

        void render_scene_texture() const {
            if (!m_viewport || !m_viewport->is_framebuffer_enabled()) {
                render_placeholder();
                return;
            }

            const auto color_texture = m_viewport->get_color_texture();
            if (!color_texture.is_valid()) {
                render_error_message();
                return;
            }

            const ImVec2 viewport_size = ImGui::GetContentRegionAvail();
            ImGui::Image(color_texture.id, viewport_size, ImVec2(1, 0), ImVec2(0, 1));
        }

        static void render_placeholder() {
            const ImVec2 center = ImGui::GetContentRegionAvail();
            ImGui::SetCursorPos(ImVec2(center.x * 0.5f, center.y * 0.5f));
            ImGui::TextDisabled("Scene viewport not available");
        }

        static void render_error_message() {
            const ImVec2 center = ImGui::GetContentRegionAvail();
            ImGui::SetCursorPos(ImVec2(center.x * 0.5f, center.y * 0.5f));
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Texture not available");
        }

        void render_gizmo_toolbar() const {
            constexpr float toolbar_offset_x = 10.0f;
            constexpr float toolbar_offset_y = 10.0f;
            constexpr float button_size = 30.0f;

            const ImVec2 button_pos(ImGui::GetWindowContentRegionMax().x - (button_size * 3 + 20),
                                    ImGui::GetWindowContentRegionMin().y + toolbar_offset_y);

            ImGui::SetCursorPos(button_pos);

            if (ImGui::Button("T", ImVec2(button_size, button_size))) {
                // Translate gizmo
            }
            ImGui::SameLine();

            if (ImGui::Button("R", ImVec2(button_size, button_size))) {
                // Rotate gizmo
            }
            ImGui::SameLine();

            if (ImGui::Button("S", ImVec2(button_size, button_size))) {
                // Scale gizmo
            }
        }

        rendering::Viewport* m_viewport = nullptr;
        bool m_focused = false;
        bool m_hovered = false;
    };

} // namespace star::editor
