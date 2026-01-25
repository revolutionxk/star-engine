#pragma once

#include <imgui.h>

namespace star::editor {

    class EditorTheme {
      public:
        static void apply_theme() {
            auto& style = ImGui::GetStyle();
            ImVec4* colors = style.Colors;

            style.WindowRounding = 8.0f;
            style.FrameRounding = 6.0f;
            style.PopupRounding = 6.0f;
            style.ScrollbarSize = 14.0f;
            style.ScrollbarRounding = 12.0f;
            style.GrabMinSize = 10.0f;
            style.GrabRounding = 6.0f;
            style.TabRounding = 6.0f;
            style.ChildRounding = 6.0f;
            style.WindowBorderSize = 0.0f;
            style.FrameBorderSize = 0.0f;
            style.PopupBorderSize = 1.0f;

            style.ItemSpacing = ImVec2(10.0f, 8.0f);
            style.FramePadding = ImVec2(10.0f, 6.0f);
            style.WindowPadding = ImVec2(12.0f, 12.0f);
            style.IndentSpacing = 20.0f;

            constexpr auto bg_darkest = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
            constexpr auto bg_dark = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
            constexpr auto bg_med = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
            constexpr auto bg_light = ImVec4(0.24f, 0.24f, 0.25f, 1.00f);

            constexpr auto accent_primary = ImVec4(0.20f, 0.55f, 0.90f, 1.00f);
            constexpr auto accent_hover = ImVec4(0.30f, 0.65f, 1.00f, 1.00f);
            constexpr auto accent_active = ImVec4(0.10f, 0.45f, 0.80f, 1.00f);

            constexpr auto text_main = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
            constexpr auto text_dim = ImVec4(0.60f, 0.60f, 0.65f, 1.00f);

            colors[ImGuiCol_Text] = text_main;
            colors[ImGuiCol_TextDisabled] = text_dim;

            colors[ImGuiCol_WindowBg] = bg_dark;
            colors[ImGuiCol_ChildBg] = bg_darkest;
            colors[ImGuiCol_PopupBg] = bg_dark;

            colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.35f, 0.40f);
            colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

            colors[ImGuiCol_FrameBg] = bg_med;
            colors[ImGuiCol_FrameBgHovered] = bg_light;
            colors[ImGuiCol_FrameBgActive] = bg_darkest;

            colors[ImGuiCol_TitleBg] = bg_darkest;
            colors[ImGuiCol_TitleBgActive] = bg_darkest;
            colors[ImGuiCol_TitleBgCollapsed] = bg_darkest;
            colors[ImGuiCol_MenuBarBg] = bg_darkest;

            colors[ImGuiCol_ScrollbarBg] = bg_darkest;
            colors[ImGuiCol_ScrollbarGrab] = bg_med;
            colors[ImGuiCol_ScrollbarGrabHovered] = bg_light;
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

            colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
            colors[ImGuiCol_SeparatorHovered] = accent_hover;
            colors[ImGuiCol_SeparatorActive] = accent_active;

            colors[ImGuiCol_ResizeGrip] = bg_med;
            colors[ImGuiCol_ResizeGripHovered] = accent_hover;
            colors[ImGuiCol_ResizeGripActive] = accent_active;

            colors[ImGuiCol_Tab] = bg_med;
            colors[ImGuiCol_TabHovered] = accent_hover;
            colors[ImGuiCol_TabActive] = accent_primary;
            colors[ImGuiCol_TabUnfocused] = bg_med;
            colors[ImGuiCol_TabUnfocusedActive] = bg_light;

            colors[ImGuiCol_DockingPreview] = accent_primary;
            colors[ImGuiCol_DockingEmptyBg] = bg_darkest;

            colors[ImGuiCol_PlotLines] = accent_primary;
            colors[ImGuiCol_PlotLinesHovered] = accent_hover;
            colors[ImGuiCol_PlotHistogram] = accent_primary;
            colors[ImGuiCol_PlotHistogramHovered] = accent_hover;

            colors[ImGuiCol_TableHeaderBg] = bg_med;
            colors[ImGuiCol_TableBorderStrong] = colors[ImGuiCol_Border];
            colors[ImGuiCol_TableBorderLight] = colors[ImGuiCol_Border];
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
