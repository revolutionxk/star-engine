#include "scene_hierarchy_panel.hpp"
#include "editor_context.hpp"
#include "editor_app.hpp"
#include "star/scene/scene.hpp"
#include "star/scene/transform.hpp"
#include <imgui.h>

namespace star::editor {
    SceneHierarchyPanel::SceneHierarchyPanel(EditorContext &context)
        : EditorPanel("Scene Hierarchy", context) {
    }

    void SceneHierarchyPanel::on_imgui_render() {
        ImGui::Begin("Scene Hierarchy", nullptr);

        auto *scene = get_context().get_app()->get_active_scene();
        if (!scene) {
            ImGui::Text("No active scene");
            ImGui::End();
            return;
        }

        if (ImGui::Button("Create Entity")) {
            auto entity = scene->create_entity();
            scene->add_component<Transform>(entity);
            get_context().set_selected_entity(entity);
        }

        ImGui::Separator();

        const auto& scene_name = scene->get_name();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_OpenOnArrow
            | ImGuiTreeNodeFlags_OpenOnDoubleClick;

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        auto opened = ImGui::TreeNodeEx(std::format("active_{}", scene_name).c_str(), flags, scene_name.c_str());

        if (opened) {
            for (const auto entity : scene->get_registry().view<Entity>()) {
                std::string name = "Entity " + std::to_string(static_cast<uint32_t>(entity));
                draw_entity_node(entity, name);
            }

            ImGui::TreePop();
        }

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
            get_context().clear_selection();
        }

        draw_context_menu();

        ImGui::End();
    }

    void SceneHierarchyPanel::draw_entity_node(Entity entity, const std::string &name) const {
        const auto selected = get_context().get_selected_entity();
        const bool is_selected = selected.has_value() && selected.value() == entity;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                                   | ImGuiTreeNodeFlags_SpanAvailWidth
                                   | ImGuiTreeNodeFlags_Leaf;

        if (is_selected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void *>(static_cast<uint64_t>(entity)),
                                              flags, "%s", name.c_str());

        if (ImGui::IsItemClicked()) {
            get_context().set_selected_entity(entity);
        }

        if (opened) {
            ImGui::TreePop();
        }
    }

    void SceneHierarchyPanel::draw_context_menu() const {
        if (ImGui::BeginPopupContextWindow(
            nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("Create Entity")) {
                if (auto *scene = get_context().get_app()->get_active_scene()) {
                    const auto entity = scene->create_entity();
                    scene->add_component<Transform>(entity);
                    get_context().set_selected_entity(entity);
                }
            }

            ImGui::EndPopup();
        }
    }
}
