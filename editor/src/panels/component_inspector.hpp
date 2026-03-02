#pragma once

#include <flecs.h>

#include "components/ui_components.hpp"
#include "star/ecs/component_registry.hpp"
#include "star/rendering/components/material_instance.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/resources/resource_manager.hpp"

namespace star::editor {
    class ComponentInspector {
      public:
        void set_resource_manager(resources::ResourceManager* rm) { m_resource_manager = rm; }

        void draw_components(const flecs::entity entity) const {
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
                if (void* ptr = ecs->get_mut_ptr(entity)) {
                    ui::draw_fields(type_info, ptr);

                    if (type_info.name == "MaterialInstance") {
                        auto* inst = static_cast<components::MaterialInstance*>(ptr);
                        const resources::Material* mat = resolve_material(entity, *inst);
                        ui::draw_material_instance(*inst, mat);
                    }
                }
                ImGui::PopID();
                ImGui::Spacing();
            }
        }

        void draw_add_popup(const flecs::entity entity) const {
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

      private:
        [[nodiscard]] const resources::Material* resolve_material(
            const flecs::entity entity, const components::MaterialInstance& inst) const {
            if (!m_resource_manager)
                return nullptr;
            if (inst.material.is_valid())
                return m_resource_manager->get_material(inst.material);
            if (const auto* mr = entity.try_get<components::MeshRenderer>())
                return m_resource_manager->get_material(mr->material);
            return nullptr;
        }

        resources::ResourceManager* m_resource_manager = nullptr;
    };

} // namespace star::editor
