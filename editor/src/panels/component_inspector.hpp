#pragma once

#include <flecs.h>

#include "components/ui_components.hpp"
#include "star/ecs/component_registry.hpp"

namespace star::editor {
    class ComponentInspector {
      public:
        static void draw_components(const flecs::entity entity) {
            for (const auto& type_registry = meta::TypeRegistry::instance();
                 const auto& type_info : type_registry.all_types()) {
                const auto* ecs = type_registry.get_extension<ecs::EcsComponentInfo>(type_info.type);
                if (!ecs || !ecs->has(entity))
                    continue;

                const bool can_remove = !ecs::has_flag(ecs->flags, ecs::RegistrationFlags::Required);

                constexpr ImGuiTreeNodeFlags header_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                                            ImGuiTreeNodeFlags_SpanAvailWidth |
                                                            ImGuiTreeNodeFlags_AllowOverlap;

                const bool open = ImGui::CollapsingHeader(type_info.name.data(), header_flags);

                if (can_remove && ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Remove Component"))
                        ecs->remove(entity);
                    ImGui::EndPopup();
                }

                if (!open)
                    continue;

                ImGui::PushID(type_info.name.data());
                if (void* ptr = ecs->get_mut_ptr(entity))
                    ui::draw_fields(type_info, ptr);
                ImGui::PopID();
                ImGui::Spacing();
            }
        }

        static void draw_add_popup(const flecs::entity entity) {
            if (ImGui::Button("Add Component", {-1.0f, 0.0f}))
                ImGui::OpenPopup("##AddComponent");

            if (!ImGui::BeginPopup("##AddComponent"))
                return;

            ImGui::TextDisabled("Components");
            ImGui::Separator();

            bool any = false;
            const auto& type_registry = meta::TypeRegistry::instance();
            for (const auto& type_info : type_registry.all_types()) {
                const auto* ecs = type_registry.get_extension<ecs::EcsComponentInfo>(type_info.type);
                if (!ecs || ecs::has_flag(ecs->flags, ecs::RegistrationFlags::Hidden))
                    continue;
                if (ecs->has(entity))
                    continue;

                if (ImGui::MenuItem(type_info.name.data())) {
                    ecs->add(entity);
                    ImGui::CloseCurrentPopup();
                }
                any = true;
            }

            if (!any)
                ImGui::TextDisabled("(all components present)");

            ImGui::EndPopup();
        }
    };

} // namespace star::editor
