#pragma once

#include <imgui.h>

namespace star::editor {

    class EditorTheme {
      public:
        static void apply_theme() {
            auto& style = ImGui::GetStyle();
            ImVec4* colors = style.Colors;

            style.WindowRounding = 6.0f;
            style.FrameRounding = 4.0f;
            style.PopupRounding = 4.0f;
            style.ScrollbarSize = 12.0f;
            style.ScrollbarRounding = 8.0f;
            style.GrabMinSize = 8.0f;
            style.GrabRounding = 4.0f;
            style.TabRounding = 6.0f;
            style.ChildRounding = 6.0f;
            style.WindowBorderSize = 1.0f;
            style.FrameBorderSize = 0.0f;
            style.PopupBorderSize = 1.0f;

            style.ItemSpacing = ImVec2(8.0f, 6.0f);
            style.FramePadding = ImVec2(8.0f, 4.0f);
            style.WindowPadding = ImVec2(10.0f, 10.0f);

            constexpr auto bg_darkest = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
            constexpr auto bg_dark = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
            constexpr auto bg_med = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
            constexpr auto bg_light = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);

            constexpr auto accent_primary = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
            constexpr auto accent_hover = ImVec4(0.36f, 0.69f, 1.00f, 1.00f);
            constexpr auto accent_active = ImVec4(0.16f, 0.49f, 0.88f, 1.00f);

            constexpr auto text_main = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
            constexpr auto text_dim = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);

            colors[ImGuiCol_Text] = text_main;
            colors[ImGuiCol_TextDisabled] = text_dim;
            colors[ImGuiCol_WindowBg] = bg_dark;
            colors[ImGuiCol_ChildBg] = bg_darkest;
            colors[ImGuiCol_PopupBg] = bg_med;
            colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
            colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_FrameBg] = bg_med;
            colors[ImGuiCol_FrameBgHovered] = bg_light;
            colors[ImGuiCol_FrameBgActive] = bg_darkest;
            colors[ImGuiCol_TitleBg] = bg_darkest;
            colors[ImGuiCol_TitleBgActive] = bg_darkest;
            colors[ImGuiCol_TitleBgCollapsed] = bg_darkest;
            colors[ImGuiCol_MenuBarBg] = bg_darkest;
            colors[ImGuiCol_ScrollbarBg] = bg_darkest;
            colors[ImGuiCol_ScrollbarGrab] = bg_light;
            colors[ImGuiCol_ScrollbarGrabHovered] = accent_hover;
            colors[ImGuiCol_ScrollbarGrabActive] = accent_active;
            colors[ImGuiCol_CheckMark] = accent_primary;
            colors[ImGuiCol_SliderGrab] = accent_primary;
            colors[ImGuiCol_SliderGrabActive] = accent_active;
            colors[ImGuiCol_Button] = bg_med;
            colors[ImGuiCol_ButtonHovered] = accent_primary;
            colors[ImGuiCol_ButtonActive] = accent_active;
            colors[ImGuiCol_Header] = bg_med;
            colors[ImGuiCol_HeaderHovered] = bg_light;
            colors[ImGuiCol_HeaderActive] = bg_darkest;
            colors[ImGuiCol_Separator] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
            colors[ImGuiCol_SeparatorHovered] = accent_hover;
            colors[ImGuiCol_SeparatorActive] = accent_active;
            colors[ImGuiCol_ResizeGrip] = bg_med;
            colors[ImGuiCol_ResizeGripHovered] = accent_hover;
            colors[ImGuiCol_ResizeGripActive] = accent_active;
            colors[ImGuiCol_Tab] = bg_med;
            colors[ImGuiCol_TabHovered] = accent_hover;
            colors[ImGuiCol_TabActive] = accent_active;
            colors[ImGuiCol_TabUnfocused] = bg_med;
            colors[ImGuiCol_TabUnfocusedActive] = bg_light;
            colors[ImGuiCol_DockingPreview] = accent_primary;
            colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
            colors[ImGuiCol_PlotLines] = accent_primary;
            colors[ImGuiCol_PlotLinesHovered] = accent_hover;
            colors[ImGuiCol_PlotHistogram] = accent_primary;
            colors[ImGuiCol_PlotHistogramHovered] = accent_hover;
            colors[ImGuiCol_TableHeaderBg] = bg_med;
            colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
            colors[ImGuiCol_TableBorderLight] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
            colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);
            colors[ImGuiCol_TextSelectedBg] = accent_primary;
            colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
            colors[ImGuiCol_NavHighlight] = accent_primary;
            colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
            colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
            colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
        }

        static bool draw_header(const char* label) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 6));
            const bool open =
                ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                             ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);
            ImGui::PopStyleVar();
            return open;
        }
    };

} // namespace star::editor
