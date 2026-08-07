#pragma once

#include <algorithm>
#include <string>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "star/core/reflection/reflect.hpp"
#include "star/rendering/components/material_instance.hpp"
#include "star/resources/material/material.hpp"

namespace star::editor::ui {
    inline float map_range(const float value, const float min_in, const float max_in, const float min_out,
                           const float max_out) {
        return min_out + (value - min_in) * (max_out - min_out) / (max_in - min_in);
    }

    inline bool begin_property_row(const char* label) {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = ImGui::GetCurrentWindow();

        if (!ImGui::BeginTable(label, 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings))
            return false;

        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 0.35f);
        ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch, 0.65f);
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);

        ImGui::TableSetColumnIndex(1);
        ImGui::PushItemWidth(-FLT_MIN);

        return true;
    }

    inline void end_property_row() {
        ImGui::PopItemWidth();
        ImGui::EndTable();
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

    struct FieldsResult {
        bool changed{false};
        bool committed{false};
        bool activated{false};

        void merge(const FieldsResult& other) noexcept {
            changed = changed || other.changed;
            committed = committed || other.committed;
            activated = activated || other.activated;
        }
    };

    inline FieldsResult capture_item_status() {
        return {ImGui::IsItemEdited() && !ImGui::IsItemActive(), ImGui::IsItemDeactivatedAfterEdit(),
                ImGui::IsItemActivated()};
    }

    inline void capture_item_status_into(FieldsResult* status) {
        if (status)
            status->merge(capture_item_status());
    }

    inline bool vector3_control(const char* label, float values[3], float reset_value = 0.f, float speed = 0.1f,
                                FieldsResult* status = nullptr) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        ImGui::PushID(label);

        ImGui::TextUnformatted(label);
        ImGui::SameLine(ImGui::GetContentRegionAvail().x * 0.3f);

        const float total_width = ImGui::GetContentRegionAvail().x;
        const float spacing = g.Style.ItemSpacing.x;
        const float block_width = (total_width - (spacing * 2.0f)) / 3.0f;

        bool value_changed = false;

        const char* axes_labels[] = {"X", "Y", "Z"};

        constexpr ImU32 bg_colors[] = {IM_COL32(180, 45, 45, 255), IM_COL32(45, 140, 45, 255),
                                       IM_COL32(35, 95, 180, 255)};

        const float frame_height = ImGui::GetFrameHeight();
        const float label_width = frame_height * 0.85f;
        const float rounding = g.Style.FrameRounding;

        for (int i = 0; i < 3; ++i) {
            ImGui::PushID(i);
            ImGui::BeginGroup();

            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, g.Style.Colors[ImGuiCol_FrameBg]);

            ImGui::SetCursorScreenPos(ImVec2(pos.x + label_width, pos.y));
            ImGui::PushItemWidth(block_width - label_width);

            if (ImGui::DragFloat("##value", &values[i], speed, 0.0f, 0.0f, "%.2f")) {
                value_changed = true;
            }

            capture_item_status_into(status);

            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                values[i] = reset_value;
                value_changed = true;
            }

            const ImU32 frame_bg_color = ImGui::GetColorU32(
                ImGui::IsItemActive() ? ImGuiCol_FrameBgActive
                                      : (ImGui::IsItemHovered() ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg));
            draw_list->AddRectFilled(ImVec2(pos.x + label_width, pos.y),
                                     ImVec2(pos.x + label_width + rounding * 2.0f, pos.y + frame_height),
                                     frame_bg_color);

            draw_list->AddRectFilled(pos, ImVec2(pos.x + label_width, pos.y + frame_height), bg_colors[i], rounding,
                                     ImDrawFlags_RoundCornersLeft);

            if (g.Style.FrameBorderSize > 0.0f) {
                const ImU32 border_color = ImGui::GetColorU32(ImGuiCol_Border);
                draw_list->AddRect(pos, ImVec2(pos.x + label_width, pos.y + frame_height), border_color, rounding,
                                   ImDrawFlags_RoundCornersLeft, g.Style.FrameBorderSize);

                draw_list->AddRect(ImVec2(pos.x + label_width, pos.y),
                                   ImVec2(pos.x + block_width, pos.y + frame_height), border_color, rounding,
                                   ImDrawFlags_RoundCornersRight, g.Style.FrameBorderSize);
            }

            const ImVec2 text_size = ImGui::CalcTextSize(axes_labels[i]);
            auto text_pos =
                ImVec2(pos.x + (label_width - text_size.x) * 0.5f, pos.y + (frame_height - text_size.y) * 0.5f);
            draw_list->AddText(text_pos, IM_COL32(245, 245, 245, 255), axes_labels[i]);

            ImGui::PopItemWidth();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            ImGui::EndGroup();

            ImGui::PopID();
            if (i < 2)
                ImGui::SameLine(0.0f, spacing);
        }

        ImGui::PopID();
        return value_changed;
    }

    inline bool float_control(const char* label, float* value, const float speed = 0.1f, const float min = 0.0f,
                              const float max = 0.0f, const char* format = "%.3f", FieldsResult* status = nullptr) {
        if (!begin_property_row(label))
            return false;

        const bool changed = ImGui::DragFloat("##value", value, speed, min, max, format);
        capture_item_status_into(status);

        end_property_row();
        return changed;
    }

    inline FieldsResult field_runtime(const star::reflection::RuntimeField& field, void* comp) {
        FieldsResult status;

        auto ref = field.get_mut(comp);
        if (!ref.is_valid())
            return status;

        const char* label = field.name.data();

        if (field.has_attr<star::reflection::attr::HideInEditor>())
            return status;

        const bool is_ro = field.has_attr<star::reflection::attr::ReadOnly>();
        if (is_ro)
            ImGui::BeginDisabled();

        const auto sp =
            field.find_attr<star::reflection::attr::Speed>().value_or(star::reflection::attr::Speed{0.1f}).value;
        const auto [min, max] =
            field.find_attr<star::reflection::attr::Range>().value_or(star::reflection::attr::Range{});
        const bool color = field.has_attr<star::reflection::attr::Color>();

        if (const auto ty = field.value_type; ty == typeid(f32)) {
            float_control(label, ref.as<f32>(), sp, min, max, "%.3f", &status);
        } else if (ty == typeid(f64)) {
            float fv = static_cast<float>(*ref.as<f64>());
            if (ImGui::DragFloat(label, &fv, sp, min, max, "%.4f"))
                *ref.as<f64>() = static_cast<f64>(fv);
            status.merge(capture_item_status());
        } else if (ty == typeid(i32)) {
            ImGui::DragInt(label, ref.as<i32>(), sp, static_cast<int>(min), static_cast<int>(max));
            status.merge(capture_item_status());
        } else if (ty == typeid(u32)) {
            int iv = static_cast<int>(*ref.as<u32>());
            if (ImGui::DragInt(label, &iv, sp, 0, static_cast<int>(max)))
                *ref.as<u32>() = static_cast<u32>(iv);
            status.merge(capture_item_status());
        } else if (ty == typeid(u8)) {
            int iv = *ref.as<u8>();
            const auto r2 =
                field.find_attr<star::reflection::attr::Range>().value_or(star::reflection::attr::Range{0.f, 255.f});
            if (ImGui::SliderInt(label, &iv, static_cast<int>(r2.min), static_cast<int>(r2.max)))
                *ref.as<u8>() = static_cast<u8>(iv);
            status.merge(capture_item_status());
        } else if (ty == typeid(bool)) {
            ImGui::Checkbox(label, ref.as<bool>());
            status.merge(capture_item_status());
        } else if (ty == typeid(std::string)) {
            char buf[512]{};
            std::strncpy(buf, ref.as<std::string>()->c_str(), sizeof(buf) - 1);
            if (ImGui::InputText(label, buf, sizeof(buf)))
                *ref.as<std::string>() = buf;
            status.merge(capture_item_status());
        } else if (ty == typeid(Vector2)) {
            auto* v = ref.as<Vector2>();
            ImGui::DragFloat2(label, &v->x, sp, min, max, "%.3f");
            status.merge(capture_item_status());
        } else if (ty == typeid(Vector3)) {
            auto* v = ref.as<Vector3>();
            if (color) {
                ImGui::ColorEdit3(label, v->data);
                status.merge(capture_item_status());
            } else {
                vector3_control(label, v->data, 0.f, sp, &status);
            }
        } else if (ty == typeid(Vector4)) {
            auto* v = ref.as<Vector4>();
            if (color)
                ImGui::ColorEdit4(label, v->data);
            else
                ImGui::DragFloat4(label, v->data, sp, min, max, "%.3f");
            status.merge(capture_item_status());
        } else if (ty == typeid(Quaternion)) {
            auto* q = ref.as<Quaternion>();
            auto euler = q->to_euler() * math::Constants<f32>::rad_to_deg;
            const auto sp2 =
                field.find_attr<star::reflection::attr::Speed>().value_or(star::reflection::attr::Speed{0.5f}).value;
            if (vector3_control(label, euler.data, 0.f, sp2, &status))
                *q = Quaternion::from_euler(radians(euler.x), radians(euler.y), radians(euler.z));
        } else if (!field.enum_labels.empty()) {
            int current = 0;
            std::memcpy(&current, ref.data, std::min(ref.data ? sizeof(int) : 0, sizeof(int)));
            std::vector<const char*> ptrs;
            ptrs.reserve(field.enum_labels.size());
            for (const auto& lbl : field.enum_labels)
                ptrs.push_back(lbl.data());
            if (ImGui::Combo(label, &current, ptrs.data(), static_cast<int>(ptrs.size())))
                std::memcpy(ref.data, &current, sizeof(int));
            status.merge(capture_item_status());
        } else {
            int iv = 0;
            std::memcpy(&iv, ref.data, sizeof(int));
            if (ImGui::DragInt(label, &iv, 1))
                std::memcpy(ref.data, &iv, sizeof(int));
            status.merge(capture_item_status());
        }

        if (is_ro)
            ImGui::EndDisabled();

        if (const auto tt = field.find_attr<star::reflection::attr::Tooltip>()) {
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                ImGui::SetTooltip("%s", tt->text.data());
        }

        return status;
    }

    inline FieldsResult fields(const star::reflection::RuntimeTypeInfo& type_info, void* comp) {
        FieldsResult result;
        ImGui::PushItemWidth(-140.0f);
        for (const auto& field : type_info.fields)
            result.merge(field_runtime(field, comp));
        ImGui::PopItemWidth();
        return result;
    }

    inline void material_instance(components::MaterialInstance& inst, const resources::Material* mat) {
        ImGui::PushItemWidth(-140.f);

        auto find = [&](std::string_view name) -> rendering::MaterialProperty* {
            for (auto& p : inst.parameters)
                if (p.name == name)
                    return &p;
            return nullptr;
        };

        auto remove = [&](std::string_view name) {
            std::erase_if(inst.parameters, [name](const auto& p) { return p.name == name; });
        };

        ImGui::SeparatorText("Parameters");
        {
            constexpr std::string_view KEY = "u_baseColor";
            auto* prop = find(KEY);
            bool enabled = prop != nullptr;

            ImGui::PushID(KEY.data());
            if (ImGui::Checkbox("##en", &enabled)) {
                if (enabled) {
                    const Vector4 def = mat ? mat->albedo_color : Vector4{1.f, 1.f, 1.f, 1.f};
                    inst.parameters.push_back({std::string(KEY), def});
                    prop = find(KEY);
                } else {
                    remove(KEY);
                    prop = nullptr;
                }
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", enabled ? "Override active" : "Using material default — check to override");

            ImGui::SameLine();
            Vector4 col =
                prop ? std::get<Vector4>(prop->value) : (mat ? mat->albedo_color : Vector4{1.f, 1.f, 1.f, 1.f});
            if (!enabled)
                ImGui::BeginDisabled();
            if (ImGui::ColorEdit4("Albedo Color", col.data) && prop)
                std::get<Vector4>(prop->value) = col;
            if (!enabled)
                ImGui::EndDisabled();
            ImGui::PopID();
        }

        {
            constexpr std::string_view KEY = "u_materialParams";
            auto* prop = find(KEY);
            bool enabled = prop != nullptr;

            ImGui::PushID(KEY.data());
            if (ImGui::Checkbox("##en", &enabled)) {
                if (enabled) {
                    const Vector4 def{mat ? mat->metallic : 0.f, mat ? mat->roughness : 0.5f, 0.f, 0.f};
                    inst.parameters.push_back({std::string(KEY), def});
                    prop = find(KEY);
                } else {
                    remove(KEY);
                    prop = nullptr;
                }
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", enabled ? "Override active" : "Using material default — check to override");

            ImGui::SameLine();
            float metallic = prop ? std::get<Vector4>(prop->value).x : (mat ? mat->metallic : 0.f);
            float roughness = prop ? std::get<Vector4>(prop->value).y : (mat ? mat->roughness : 0.5f);

            if (!enabled)
                ImGui::BeginDisabled();
            ImGui::BeginGroup();
            ImGui::Text("Metallic / Roughness");
            bool changed = ImGui::SliderFloat("Metallic##mp", &metallic, 0.f, 1.f);
            changed |= ImGui::SliderFloat("Roughness##mp", &roughness, 0.f, 1.f);
            ImGui::EndGroup();
            if (!enabled)
                ImGui::EndDisabled();

            if (changed && prop) {
                auto& v = std::get<Vector4>(prop->value);
                v.x = metallic;
                v.y = roughness;
            }
            ImGui::PopID();
        }

        bool has_custom = false;
        for (int i = 0; i < static_cast<int>(inst.parameters.size()); ++i) {
            auto& prop = inst.parameters[i];
            if (prop.name == "u_baseColor" || prop.name == "u_materialParams")
                continue;

            if (!has_custom) {
                ImGui::SeparatorText("Custom");
                has_custom = true;
            }

            ImGui::PushID(i);
            const bool is_color = prop.name.find("olor") != std::string::npos;
            std::visit(
                [&]<typename T>(T& v) {
                    using V = std::decay_t<T>;
                    if constexpr (std::is_same_v<V, float>)
                        ImGui::DragFloat(prop.name.c_str(), &v, 0.01f);
                    else if constexpr (std::is_same_v<V, Vector2>)
                        ImGui::DragFloat2(prop.name.c_str(), &v.x, 0.01f);
                    else if constexpr (std::is_same_v<V, Vector3>)
                        is_color ? (void)ImGui::ColorEdit3(prop.name.c_str(), v.data)
                                 : (void)ImGui::DragFloat3(prop.name.c_str(), v.data, 0.01f);
                    else if constexpr (std::is_same_v<V, Vector4>)
                        is_color ? (void)ImGui::ColorEdit4(prop.name.c_str(), v.data)
                                 : (void)ImGui::DragFloat4(prop.name.c_str(), v.data, 0.01f);
                },
                prop.value);
            ImGui::SameLine();
            if (ImGui::SmallButton("x"))
                inst.parameters.erase(inst.parameters.begin() + i--);
            ImGui::PopID();
        }

        ImGui::Spacing();
        if (ImGui::Button("+ Add Custom Uniform", {-1.f, 0.f}))
            ImGui::OpenPopup("##AddCustomMI");

        if (ImGui::BeginPopup("##AddCustomMI")) {
            static char custom_name[128]{};
            ImGui::InputText("Uniform name", custom_name, sizeof(custom_name));
            if (ImGui::MenuItem("Add as Vec4 (color)") && custom_name[0]) {
                inst.parameters.push_back({custom_name, Vector4{1.f, 1.f, 1.f, 1.f}});
                custom_name[0] = '\0';
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Add as float") && custom_name[0]) {
                inst.parameters.push_back({custom_name, 0.f});
                custom_name[0] = '\0';
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::PopItemWidth();
    }
} // namespace star::editor::ui
