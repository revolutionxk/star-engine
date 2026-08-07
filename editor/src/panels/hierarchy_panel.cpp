#include "hierarchy_panel.hpp"

#include "../core/file_dialog.hpp"

#include <imgui.h>

#include <memory>

#include "../core/commands/entity_commands.hpp"
#include "../core/icon_registry.hpp"
#include "core/gltf_import.hpp"
#include "star/ecs/components/camera.hpp"
#include "star/rendering/components/light.hpp"
#include "star/rendering/components/mesh_renderer.hpp"

namespace star::editor {
    struct EntityVisual {
        const char* icon;
        ImVec4 tint;
    };

    EntityVisual visual_for_entity(const flecs::entity entity) {
        if (entity.has<components::Camera>())
            return {"camera", ImVec4(0.45f, 0.78f, 0.95f, 1.0f)};
        if (entity.has<components::Light>())
            return {"lightbulb", ImVec4(0.98f, 0.82f, 0.4f, 1.0f)};
        if (entity.has<components::MeshRenderer>())
            return {"box", ImVec4(0.72f, 0.62f, 1.0f, 1.0f)};
        return {"circle-dashed", ImVec4(0.62f, 0.62f, 0.66f, 1.0f)};
    }

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
                                                 node_flags, "         %s", entity.name().c_str());

        const auto [icon, tint] = visual_for_entity(entity);
        const ImVec2 rect_min = ImGui::GetItemRectMin();
        const float row_h = ImGui::GetItemRectSize().y;
        const float icon_sz = ImGui::GetTextLineHeight();
        const float icon_x = rect_min.x + ImGui::GetTreeNodeToLabelSpacing();
        const float icon_y = rect_min.y + (row_h - icon_sz) * 0.5f;
        ImGui::GetWindowDrawList()->AddImage(m_editor_window->icons().icon(icon), {icon_x, icon_y},
                                             {icon_x + icon_sz, icon_y + icon_sz}, {0, 0}, {1, 1},
                                             ImGui::ColorConvertFloat4ToU32(tint));

        if (ImGui::IsItemClicked()) {
            m_selected_entity = entity;
            on_entity_selected(m_selected_entity);
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete Entity") && m_command_stack)
                m_command_stack->push(std::make_unique<DestroyEntityCommand>(entity, "Delete Entity"));
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
            if (ImGui::MenuItem("Import glTF..."))
                import_gltf_model();
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

    void HierarchyPanel::create_empty_entity() {
        if (!m_editor_window || !m_command_stack)
            return;
        auto* scene = m_editor_window->scene_manager().get_active_scene();
        if (!scene)
            return;
        m_command_stack->push(std::make_unique<CreateEntityCommand>(scene->world().native(), std::string{},
                                                                     scene->root(), "Create Entity"));
    }

    void HierarchyPanel::create_camera_entity() {
        if (!m_editor_window || !m_command_stack)
            return;
        auto* scene = m_editor_window->scene_manager().get_active_scene();
        if (!scene)
            return;
        m_command_stack->push(std::make_unique<CreateEntityCommand>(scene->world().native(), "Camera", scene->root(),
                                                                     "Create Camera"));
    }

    void HierarchyPanel::import_gltf_model() const {
        if (!m_editor_window)
            return;
        m_editor_window->open_file_dialog(FileRequest::Model);
    }

    void HierarchyPanel::on_entity_selected(flecs::entity& entity) {
        EditorEvent event;
        event.type = EditorEventType::EntitySelected;

        auto event_data = NodeSelectedEvent{entity};
        event.data = &event_data;

        EditorEventBus::instance().publish(event);
    }
} // namespace star::editor
