#include "hierarchy_panel.hpp"

#include <imgui.h>

namespace star::editor {
    void HierarchyPanel::on_imgui_render() {
        if (!m_is_open)
            return;

        ImGui::Begin(m_name.c_str(), &m_is_open);

        render_search_bar();
        ImGui::Separator();

        render_entity_list();
        render_context_menu();

        ImGui::End();
    }

    void HierarchyPanel::render_search_bar() {
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputTextWithHint("##Search", "Search entities...", m_search_buffer.data(),
                                     m_search_buffer.size())) {
            filter_entities();
        }
    }

    void HierarchyPanel::render_entity_list() {
        ImGui::BeginChild("EntityList", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        const auto& scene = m_editor_window->scene_manager();
        const auto active_scene = scene.get_active_scene();

        if (!active_scene) {
            ImGui::EndChild();
            return;
        }

        const auto root = active_scene->root();
        active_scene->world().native().defer_begin();
        root.children([this](const flecs::entity child) { render_entity_node(child); });
        active_scene->world().native().defer_end();

        ImGui::EndChild();
    }

    void HierarchyPanel::render_entity_node(const flecs::entity entity) {
        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

        if (entity.world().count(flecs::ChildOf, entity.id()) == 0)
            node_flags |= ImGuiTreeNodeFlags_Leaf;

        if (entity == m_selected_entity)
            node_flags |= ImGuiTreeNodeFlags_Selected;

        const bool node_open = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(entity.id())),
                                                 node_flags, "%s", entity.name().c_str());
        if (ImGui::IsItemClicked()) {
            m_selected_entity = entity;
            on_entity_selected(m_selected_entity);
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete Entity"))
                entity.destruct();
            ImGui::EndPopup();
        }

        if (node_open) {
            entity.children([this](const flecs::entity child) { render_entity_node(child); });
            ImGui::TreePop();
        }
    }

    void HierarchyPanel::render_context_menu() {
        if (ImGui::BeginPopupContextWindow("HierarchyContextMenu",
                                           ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("Create Empty Entity"))
                create_empty_entity();
            if (ImGui::MenuItem("Create Camera"))
                create_camera_entity();
            ImGui::Separator();
            if (ImGui::MenuItem("Paste")) {
                // TODO: paste logic
            }
            ImGui::EndPopup();
        }
    }

    void HierarchyPanel::filter_entities() {
        // TODO: filter when scene integration is complete
    }

    void HierarchyPanel::create_empty_entity() const {
        if (!m_editor_window)
            return;
        auto* scene = m_editor_window->scene_manager().get_active_scene();
        if (!scene)
            return;
        auto entity = scene->world().native().entity();
        entity.child_of(scene->root());
        STAR_LOG_INFO(LogCategory::Editor, "Created empty entity '{}'", entity.name().c_str());
    }

    void HierarchyPanel::create_camera_entity() const {
        if (!m_editor_window)
            return;
        auto* scene = m_editor_window->scene_manager().get_active_scene();
        if (!scene)
            return;
        auto entity = scene->world().native().entity("Camera");
        entity.child_of(scene->root());
        STAR_LOG_INFO(LogCategory::Editor, "Created camera entity");
    }

    void HierarchyPanel::on_entity_selected(flecs::entity& entity) {
        EditorEvent event;
        event.type = EditorEventType::EntitySelected;

        auto event_data = NodeSelectedEvent{entity};
        event.data = &event_data;

        EditorEventBus::instance().publish(event);
    }
} // namespace star::editor
