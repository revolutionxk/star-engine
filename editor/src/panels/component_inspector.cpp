#include "component_inspector.hpp"

#include <string>
#include <string_view>

#include <imgui.h>

#include "../core/commands/entity_commands.hpp"
#include "../core/icon_registry.hpp"

namespace star::editor {
    struct ComponentVisual {
        const char* icon;
        ImVec4 tint;
    };

    ComponentVisual visual_for_component(const std::string_view name) {
        if (name == "Transform")
            return {"axis-3d", ImVec4(0.85f, 0.85f, 0.9f, 1.0f)};
        if (name == "MeshRenderer")
            return {"box", ImVec4(0.72f, 0.62f, 1.0f, 1.0f)};
        if (name == "MaterialInstance")
            return {"palette", ImVec4(1.0f, 0.68f, 0.35f, 1.0f)};
        if (name == "Camera")
            return {"camera", ImVec4(0.45f, 0.78f, 0.95f, 1.0f)};
        if (name == "Light")
            return {"lightbulb", ImVec4(0.98f, 0.82f, 0.4f, 1.0f)};
        if (name == "Atmosphere")
            return {"sun", ImVec4(0.95f, 0.78f, 0.5f, 1.0f)};
        if (name == "RigidBody" || name == "Collider")
            return {"component", ImVec4(0.6f, 0.85f, 0.7f, 1.0f)};
        return {"component", ImVec4(0.78f, 0.78f, 0.82f, 1.0f)};
    }

    void ComponentInspector::draw_components(const flecs::entity entity) const {
        const auto& type_registry = star::reflection::TypeRegistry::instance();
        for (const auto& type_info : type_registry.all_types()) {
            const auto* ecs = type_registry.get_extension<ecs::EcsComponentInfo>(type_info.type);
            if (!ecs || !ecs->has(entity))
                continue;

            const bool can_remove = !ecs::has_flag(ecs->flags, ecs::RegistrationFlags::Required);

            constexpr ImGuiTreeNodeFlags header_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                                        ImGuiTreeNodeFlags_SpanAvailWidth |
                                                        ImGuiTreeNodeFlags_AllowOverlap;

            const ComponentVisual visual = visual_for_component(type_info.name);
            const std::string header_label = "     " + std::string(type_info.name);
            const bool open = ImGui::CollapsingHeader(header_label.c_str(), header_flags);

            if (m_icons) {
                const ImVec2 rect_min = ImGui::GetItemRectMin();
                const float row_h = ImGui::GetItemRectSize().y;
                const float icon_sz = ImGui::GetTextLineHeight();
                const float icon_x = rect_min.x + ImGui::GetTreeNodeToLabelSpacing();
                const float icon_y = rect_min.y + (row_h - icon_sz) * 0.5f;
                ImGui::GetWindowDrawList()->AddImage(m_icons->icon(visual.icon), {icon_x, icon_y},
                                                     {icon_x + icon_sz, icon_y + icon_sz}, {0, 0}, {1, 1},
                                                     ImGui::ColorConvertFloat4ToU32(visual.tint));
            }

            if (can_remove && ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Remove Component") && m_command_stack)
                    m_command_stack->push(std::make_unique<RemoveComponentCommand>(entity, type_info.type,
                                                                                    std::string{type_info.name}));
                ImGui::EndPopup();
            }

            if (!open)
                continue;

            ImGui::PushID(type_info.name.data());
            if (void* ptr = ecs->get_mut_ptr(entity)) {
                std::any before = clone_component(entity, type_info.type);
                const ui::FieldsResult result = ui::fields(type_info, ptr);

                if (result.committed && before.has_value()) {
                    std::any after = clone_component(entity, type_info.type);
                    if (after.has_value() && m_command_stack)
                        m_command_stack->push(std::make_unique<SetComponentCommand>(
                            entity, type_info.type, std::move(before), std::move(after),
                            std::string{type_info.name}));
                }

                if (type_info.name == "MaterialInstance") {
                    auto* inst = static_cast<components::MaterialInstance*>(ptr);
                    const resources::Material* mat = resolve_material(entity, *inst);
                    ui::material_instance(*inst, mat);
                }
            }
            ImGui::PopID();
            ImGui::Spacing();
        }
    }

    void ComponentInspector::draw_add_popup(const flecs::entity entity) const {
        if (ImGui::Button("Add Component", {-1.0f, 0.0f}))
            ImGui::OpenPopup("##AddComponent");

        if (!ImGui::BeginPopup("##AddComponent"))
            return;

        ImGui::TextDisabled("Components");
        ImGui::Separator();

        bool any = false;
        const auto& type_registry = star::reflection::TypeRegistry::instance();
        for (const auto& type_info : type_registry.all_types()) {
            const auto* ecs = type_registry.get_extension<ecs::EcsComponentInfo>(type_info.type);
            if (!ecs || ecs::has_flag(ecs->flags, ecs::RegistrationFlags::Hidden))
                continue;
            if (ecs->has(entity))
                continue;

            if (ImGui::MenuItem(type_info.name.data())) {
                if (m_command_stack)
                    m_command_stack->push(std::make_unique<AddComponentCommand>(entity, type_info.type,
                                                                                 std::string{type_info.name}));
                ImGui::CloseCurrentPopup();
            }
            any = true;
        }

        if (!any)
            ImGui::TextDisabled("(all components present)");

        ImGui::EndPopup();
    }

    const resources::Material* ComponentInspector::resolve_material(const flecs::entity entity,
                                                                    const components::MaterialInstance& inst) const {
        if (!m_resource_manager)
            return nullptr;
        if (inst.material.is_valid())
            return m_resource_manager->get_material(inst.material);
        if (const auto* mr = entity.try_get<components::MeshRenderer>())
            return m_resource_manager->get_material(mr->material);
        return nullptr;
    }
} // namespace star::editor
