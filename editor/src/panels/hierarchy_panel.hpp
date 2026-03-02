#pragma once

#include <array>

#include <imgui.h>

#include "../core/editor_events.hpp"
#include "panel.hpp"
#include "star/scene/scene.hpp"
#include "star/scene/scene_manager.hpp"

namespace star::editor {
    class HierarchyPanel final : public Panel {
      public:
        HierarchyPanel(EditorWindow* editor_window) : Panel("Hierarchy"), m_editor_window(editor_window) {}

        void on_imgui_render() override {
            if (!m_is_open)
                return;

            ImGui::Begin(m_name.c_str(), &m_is_open);

            render_search_bar();
            ImGui::Separator();

            render_entity_list();
            render_context_menu();

            ImGui::End();
        }

      private:
        void render_search_bar() {
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputTextWithHint("##Search", "Search entities...", m_search_buffer.data(),
                                         m_search_buffer.size())) {
                filter_entities();
            }
        }

        void render_entity_list() {
            ImGui::BeginChild("EntityList", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            const auto& scene = m_editor_window->scene_manager();
            const auto active_scene = scene.get_active_scene();

            if (!active_scene) {
                ImGui::EndChild();
                return;
            }

            const auto root = active_scene->root();
            active_scene->world().defer_begin();
            root.children([this](const flecs::entity child) { render_entity_node(child); });
            active_scene->world().defer_end();

            ImGui::EndChild();
        }

        void render_entity_node(const flecs::entity entity) {
            ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

            if (const auto children_count = entity.world().count(flecs::ChildOf, entity.id()); children_count == 0) {
                node_flags |= ImGuiTreeNodeFlags_Leaf;
            }

            if (entity == m_selected_entity) {
                node_flags |= ImGuiTreeNodeFlags_Selected;
            }

            const bool node_open = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(entity.id())),
                                                     node_flags, "%s", entity.name().c_str());
            if (ImGui::IsItemClicked()) {
                m_selected_entity = entity;
                on_entity_selected(m_selected_entity);
            }

            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Delete Entity")) {
                    auto name = entity.name();
                    entity.destruct();
                }
                ImGui::EndPopup();
            }

            if (node_open) {
                entity.children([this](const flecs::entity child) { render_entity_node(child); });
                ImGui::TreePop();
            }
        }

        void render_context_menu() {
            if (ImGui::BeginPopupContextWindow("HierarchyContextMenu",
                                               ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
                if (ImGui::MenuItem("Create Empty Entity")) {
                    create_empty_entity();
                }
                if (ImGui::MenuItem("Create Camera")) {
                    create_camera_entity();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Paste")) {
                    // Paste logic
                }
                ImGui::EndPopup();
            }
        }

        void filter_entities() {
            // Filter logic will go here when scene integration is complete
        }

        void create_empty_entity() {
            STAR_LOG_INFO(LogCategory::Editor, "Creating empty entity");
            // Entity creation logic
        }

        void create_camera_entity() {
            STAR_LOG_INFO(LogCategory::Editor, "Creating camera entity");
            // Camera entity creation logic
        }

        static void on_entity_selected(flecs::entity& entity) {
            EditorEvent event;
            event.type = EditorEventType::EntitySelected;

            auto event_data = NodeSelectedEvent{entity};
            event.data = &event_data;

            EditorEventBus::instance().publish(event);
        }

        std::array<char, 256> m_search_buffer{};
        flecs::entity m_selected_entity;
        EditorWindow* m_editor_window = nullptr;
    };

} // namespace star::editor
