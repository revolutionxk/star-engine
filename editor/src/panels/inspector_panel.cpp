#include "inspector_panel.hpp"

#include <filesystem>
#include <memory>

#include <imgui.h>

#include "../core/commands/entity_commands.hpp"
#include "star/core/logger.hpp"
#include "star/resources/material/material.hpp"
#include "star/resources/resource_manager.hpp"

namespace star::editor {
    void InspectorPanel::on_attach() {
        EditorEventBus::instance().subscribe(EditorEventType::EntitySelected, [this](const EditorEvent& event) {
            if (const auto* data = event.get_data<NodeSelectedEvent>()) {
                m_selected_entity = data->node;
                m_selected_material.reset();
            }
        });
        EditorEventBus::instance().subscribe(EditorEventType::MaterialAssetSelected, [this](const EditorEvent& event) {
            if (const auto* path = event.get_data<std::string>()) {
                m_selected_material = *path;
                m_selected_entity.reset();
            }
        });
    }

    void InspectorPanel::on_imgui_render() {
        if (!m_is_open)
            return;

        ImGui::Begin(m_name.c_str(), &m_is_open);

        if (m_selected_material.has_value())
            render_material_editor();
        else if (has_valid_entity())
            render_entity_contents();
        else
            render_empty_state();

        ImGui::End();
    }

    bool InspectorPanel::has_valid_entity() const {
        return m_selected_entity.has_value() && m_selected_entity.value();
    }

    void InspectorPanel::render_entity_contents() const {
        const auto entity = *m_selected_entity;
        render_entity_header(entity);

        ImGui::Separator();
        ImGui::Spacing();

        m_inspector->draw_components(entity);

        ImGui::Spacing();
        m_inspector->draw_add_popup(entity);
    }

    void InspectorPanel::render_material_editor() const {
        auto* rm = m_inspector->resource_manager();
        const std::string& path = *m_selected_material;

        ImGui::TextDisabled("Material  ");
        ImGui::SameLine();
        ImGui::TextUnformatted(std::filesystem::path(path).stem().string().c_str());
        ImGui::Separator();
        ImGui::Spacing();

        if (!rm) {
            ImGui::TextDisabled("No resource manager.");
            return;
        }

        const auto handle = rm->get_or_load_material(path);
        auto* material = rm->get_material(handle);
        if (!material) {
            ImGui::TextDisabled("Could not load '%s'.", path.c_str());
            return;
        }
        
        ImGui::ColorEdit4("Albedo", &material->albedo_color.x);
        ImGui::SliderFloat("Metallic", &material->metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &material->roughness, 0.0f, 1.0f);
        ImGui::ColorEdit4("Emissive", &material->emissive_color.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

        const std::string texture = rm->texture_name(material->albedo_texture);
        ImGui::Spacing();
        ImGui::TextDisabled("Albedo Texture: %s", texture.empty() ? "(none)" : texture.c_str());

        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::Button("Save", ImVec2(120.0f, 0.0f))) {
            if (rm->save_material(handle, path))
                STAR_LOG_INFO(LogCategory::Editor, "Saved material '{}'", path);
        }
    }

    void InspectorPanel::render_entity_header(const flecs::entity entity) const {
        ImGui::TextDisabled("Entity  ");
        ImGui::SameLine();

        static char name_buf[256]{};
        std::strncpy(name_buf, entity_name(entity).c_str(), sizeof(name_buf) - 1);
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::InputText("##name", name_buf, sizeof(name_buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (m_command_stack)
                m_command_stack->push(std::make_unique<RenameEntityCommand>(entity, name_buf, "Rename Entity"));
            else
                entity.set_name(name_buf);
            STAR_LOG_INFO(LogCategory::Editor, "Renamed entity to '{}'", name_buf);
        }
    }

    void InspectorPanel::render_empty_state() {
        const float w = ImGui::GetWindowWidth();
        const float h = ImGui::GetWindowHeight();
        constexpr auto msg = "Nothing selected";
        const float tw = ImGui::CalcTextSize(msg).x;
        ImGui::SetCursorPos({(w - tw) * 0.5f, h * 0.5f});
        ImGui::TextDisabled("%s", msg);
    }
} // namespace star::editor
