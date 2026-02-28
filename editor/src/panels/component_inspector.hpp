#pragma once

#include <functional>
#include <string_view>
#include <vector>

#include <flecs.h>
#include <imgui.h>

#include "star/core/meta/visitors/imgui_draw.hpp"
#include "star/ecs/component_registry.hpp"

namespace star::editor {
    class ComponentInspector {
      public:
        template<meta::Reflected T>
        void register_draw() {
            m_draw_entries.push_back({
                meta::type_name<T>().data(),
                [](flecs::entity ent) {
                    if (auto* comp = ent.try_get_mut<T>())
                        meta::draw_component(*comp);
                },
            });
        }

        template<meta::Reflected... Ts>
        void register_all() {
            (register_draw<Ts>(), ...);
        }

        void draw_components(const flecs::entity entity, const ecs::ComponentRegistry& registry) const {
            for (const auto& entry : registry.entries()) {
                if (!entry.has(entity))
                    continue;

                const bool can_remove = !ecs::has_flag(entry.flags, ecs::RegistrationFlags::Required);

                constexpr ImGuiTreeNodeFlags header_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                                            ImGuiTreeNodeFlags_SpanAvailWidth |
                                                            ImGuiTreeNodeFlags_AllowOverlap;

                const bool open = ImGui::CollapsingHeader(entry.type_name, header_flags);

                if (can_remove && ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Remove Component"))
                        entry.remove(entity);
                    ImGui::EndPopup();
                }

                if (!open)
                    continue;

                ImGui::PushID(entry.type_name);
                if (const auto* draw = find_draw(entry.type_name))
                    draw->draw_fields(entity);
                ImGui::PopID();
                ImGui::Spacing();
            }
        }

        static void draw_add_popup(const flecs::entity entity, const ecs::ComponentRegistry& registry) {
            if (ImGui::Button("Add Component", {-1.0f, 0.0f}))
                ImGui::OpenPopup("##AddComponent");

            if (!ImGui::BeginPopup("##AddComponent"))
                return;

            ImGui::TextDisabled("Components");
            ImGui::Separator();

            bool any = false;
            for (const auto& entry : registry.entries()) {
                if (ecs::has_flag(entry.flags, ecs::RegistrationFlags::Hidden))
                    continue;
                if (entry.has(entity))
                    continue;

                if (ImGui::MenuItem(entry.type_name)) {
                    entry.add(entity);
                    ImGui::CloseCurrentPopup();
                }
                any = true;
            }

            if (!any)
                ImGui::TextDisabled("(all components present)");

            ImGui::EndPopup();
        }

      private:
        struct DrawEntry {
            const char* type_name{};
            std::function<void(flecs::entity)> draw_fields;
        };

        [[nodiscard]] const DrawEntry* find_draw(const std::string_view name) const noexcept {
            for (const auto& d : m_draw_entries)
                if (d.type_name == name)
                    return &d;
            return nullptr;
        }

        std::vector<DrawEntry> m_draw_entries;
    };

} // namespace star::editor
