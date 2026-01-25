#pragma once

#include <string>

#include <imgui.h>
#include <imgui_internal.h>

namespace star::editor::ui {
    inline float map_range(const float value, const float min_in, const float max_in, const float min_out,
                           const float max_out) {
        return min_out + (value - min_in) * (max_out - min_out) / (max_in - min_in);
    }

    inline bool toggle(const char* label, bool* v) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        const auto& g = *GImGui;
        const auto& style = g.Style;
        const auto id = window->GetID(label);
        const auto label_size = ImGui::CalcTextSize(label, nullptr, true);

        const auto height = ImGui::GetFrameHeight();
        const auto width = height * 1.55f;
        const auto radius = height * 0.50f;

        const auto pos = window->DC.CursorPos;
        const ImRect total_bb(
            pos, ImVec2(pos.x + width + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f),
                        pos.y + label_size.y + style.FramePadding.y * 2.0f));

        ImGui::ItemSize(total_bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(total_bb, id))
            return false;

        bool hovered, held;
        const bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
        if (pressed) {
            *v = !*v;
            ImGui::MarkItemEdited(id);
        }

        float t = *v ? 1.0f : 0.0f;
        if (g.LastActiveId == id) {
            if (float t_anim = ImGui::GetStateStorage()->GetFloat(id, t); t_anim != t) {
                constexpr float anim_speed = 0.08f;
                const float t_diff = t - t_anim;
                t_anim += t_diff * anim_speed;
                if (ImAbs(t_diff) < 0.005f)
                    t_anim = t;
                ImGui::GetStateStorage()->SetFloat(id, t_anim);
                t = t_anim;
            }
        } else {
            ImGui::GetStateStorage()->SetFloat(id, t);
        }

        const ImVec2 check_bb(pos.x, pos.y);
        auto bg_col = ImGui::GetColorU32(hovered && held ? ImGuiCol_FrameBgActive
                                         : hovered       ? ImGuiCol_FrameBgHovered
                                                         : ImGuiCol_FrameBg);
        const ImVec4 active_color = style.Colors[ImGuiCol_CheckMark];
        if (*v) {
            bg_col = ImGui::GetColorU32(ImVec4(active_color.x, active_color.y, active_color.z, 0.6f + (t * 0.4f)));
        }

        window->DrawList->AddRectFilled(check_bb, ImVec2(check_bb.x + width, check_bb.y + height), bg_col,
                                        height * 0.5f);

        const auto knob_radius = radius - 2.0f;
        auto knob_x = pos.x + radius + (width - radius * 2.0f) * t - (radius - 2.0f);
        const auto knob_pos_x = pos.x + 2.0f + (width - 4.0f - (knob_radius * 2.0f)) * t;

        window->DrawList->AddCircleFilled(ImVec2(knob_pos_x + knob_radius, pos.y + radius), knob_radius,
                                          IM_COL32(255, 255, 255, 255));

        if (label_size.x > 0.0f)
            ImGui::RenderText(ImVec2(pos.x + width + style.ItemInnerSpacing.x, pos.y + style.FramePadding.y), label);

        return pressed;
    }

    inline bool button(const char* label, const ImVec2& size_arg = ImVec2(0, 0)) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        const auto& g = *GImGui;
        const auto& style = g.Style;
        const auto id = window->GetID(label);
        const auto label_size = ImGui::CalcTextSize(label, nullptr, true);

        const auto pos = window->DC.CursorPos;
        const auto size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f,
                                              label_size.y + style.FramePadding.y * 2.0f);

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(size, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        bool hovered, held;
        const auto pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

        const auto col = ImGui::GetColorU32(held && hovered ? ImGuiCol_ButtonActive
                                            : hovered       ? ImGuiCol_ButtonHovered
                                                            : ImGuiCol_Button);
        const float rounding = style.FrameRounding;

        window->DrawList->AddRectFilled(bb.Min, bb.Max, col, rounding);

        if (hovered) {
            window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_BorderShadow), rounding, 0, 2.0f);
        }

        ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, nullptr, &label_size,
                                 style.ButtonTextAlign, &bb);

        return pressed;
    }

} // namespace star::editor::ui
