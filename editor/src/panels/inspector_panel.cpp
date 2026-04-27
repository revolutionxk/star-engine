#include "inspector_panel.hpp"

#include <imgui.h>

#include "star/core/logger.hpp"

namespace star::editor {
    void InspectorPanel::on_attach() {
        EditorEventBus::instance().subscribe(EditorEventType::EntitySelected, [this](const EditorEvent& event) {
            if (const auto* data = event.get_data<NodeSelectedEvent>())
                m_selected_entity = data->node;
        });
    }

    void InspectorPanel::on_imgui_render() {
        if (!m_is_open)
            return;

        ImGui::Begin(m_name.c_str(), &m_is_open);

        if (has_valid_entity())
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

    void InspectorPanel::render_entity_header(const flecs::entity entity) {
        ImGui::TextDisabled("Entity  ");
        ImGui::SameLine();

        static char name_buf[256]{};
        std::strncpy(name_buf, entity.name().c_str(), sizeof(name_buf) - 1);
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::InputText("##name", name_buf, sizeof(name_buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
            entity.set_name(name_buf);
            STAR_LOG_INFO(LogCategory::Editor, "Renamed entity to '{}'", name_buf);
        }
    }

    void InspectorPanel::render_empty_state() {
        const float w = ImGui::GetWindowWidth();
        const float h = ImGui::GetWindowHeight();
        constexpr auto msg = "No entity selected";
        const float tw = ImGui::CalcTextSize(msg).x;
        ImGui::SetCursorPos({(w - tw) * 0.5f, h * 0.5f});
        ImGui::TextDisabled("%s", msg);
    }
} // namespace star::editor
