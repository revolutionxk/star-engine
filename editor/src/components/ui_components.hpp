#pragma once

#include <algorithm>
#include <string>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "reflection/imgui_visitor.hpp"
#include "star/core/meta/reflect.hpp"
#include "star/rendering/components/material_instance.hpp"
#include "star/resources/material/material.hpp"

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

    inline void draw_field_runtime(const meta::RuntimeField& field, void* comp) {
        auto ref = field.get_mut(comp);
        if (!ref.is_valid())
            return;

        const char* label = field.name.data();

        if (field.has_attr<meta::attr::HideInEditor>())
            return;

        const bool is_ro = field.has_attr<meta::attr::ReadOnly>();
        if (is_ro)
            ImGui::BeginDisabled();

        const auto sp = field.find_attr<meta::attr::Speed>().value_or(meta::attr::Speed{0.1f}).value;
        const auto [min, max] = field.find_attr<meta::attr::Range>().value_or(meta::attr::Range{});
        const bool color = field.has_attr<meta::attr::Color>();

        if (const auto ty = field.value_type; ty == typeid(f32)) {
            ImGui::DragFloat(label, ref.as<f32>(), sp, min, max, "%.3f");
        } else if (ty == typeid(f64)) {
            float fv = static_cast<float>(*ref.as<f64>());
            if (ImGui::DragFloat(label, &fv, sp, min, max, "%.4f"))
                *ref.as<f64>() = static_cast<f64>(fv);
        } else if (ty == typeid(i32)) {
            ImGui::DragInt(label, ref.as<i32>(), sp, static_cast<int>(min), static_cast<int>(max));
        } else if (ty == typeid(u32)) {
            int iv = static_cast<int>(*ref.as<u32>());
            if (ImGui::DragInt(label, &iv, sp, 0, static_cast<int>(max)))
                *ref.as<u32>() = static_cast<u32>(iv);
        } else if (ty == typeid(u8)) {
            int iv = *ref.as<u8>();
            const auto r2 = field.find_attr<meta::attr::Range>().value_or(meta::attr::Range{0.f, 255.f});
            if (ImGui::SliderInt(label, &iv, static_cast<int>(r2.min), static_cast<int>(r2.max)))
                *ref.as<u8>() = static_cast<u8>(iv);
        } else if (ty == typeid(bool)) {
            ImGui::Checkbox(label, ref.as<bool>());
        } else if (ty == typeid(std::string)) {
            char buf[512]{};
            std::strncpy(buf, ref.as<std::string>()->c_str(), sizeof(buf) - 1);
            if (ImGui::InputText(label, buf, sizeof(buf)))
                *ref.as<std::string>() = buf;
        } else if (ty == typeid(Vector2)) {
            auto* v = ref.as<Vector2>();
            ImGui::DragFloat2(label, &v->x, sp, min, max, "%.3f");
        } else if (ty == typeid(Vector3)) {
            auto* v = ref.as<Vector3>();
            if (color)
                ImGui::ColorEdit3(label, v->data);
            else
                ImGui::DragFloat3(label, v->data, sp, min, max, "%.3f");
        } else if (ty == typeid(Vector4)) {
            auto* v = ref.as<Vector4>();
            if (color)
                ImGui::ColorEdit4(label, v->data);
            else
                ImGui::DragFloat4(label, v->data, sp, min, max, "%.3f");
        } else if (ty == typeid(Quaternion)) {
            auto* q = ref.as<Quaternion>();
            auto euler = q->to_euler() * math::Constants<f32>::rad_to_deg;
            const auto sp2 = field.find_attr<meta::attr::Speed>().value_or(meta::attr::Speed{0.5f}).value;
            if (ImGui::DragFloat3(label, euler.data, sp2, 0.f, 0.f, "%.2f deg"))
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
        } else {
            int iv = 0;
            std::memcpy(&iv, ref.data, sizeof(int));
            if (ImGui::DragInt(label, &iv, 1))
                std::memcpy(ref.data, &iv, sizeof(int));
        }

        if (is_ro)
            ImGui::EndDisabled();

        if (const auto tt = field.find_attr<meta::attr::Tooltip>()) {
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                ImGui::SetTooltip("%s", tt->text.data());
        }
    }

    inline void draw_fields(const meta::RuntimeTypeInfo& type_info, void* comp) {
        ImGui::PushItemWidth(-140.0f);
        for (const auto& field : type_info.fields)
            draw_field_runtime(field, comp);
        ImGui::PopItemWidth();
    }

    template<meta::Reflected T>
    void draw_component(T& component) {
        ImGui::PushItemWidth(-140.0f);
        meta::for_each_field(component, reflection::ImGuiVisitor{});
        ImGui::PopItemWidth();
    }

    inline void draw_material_instance(components::MaterialInstance& inst, const resources::Material* mat) {
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
