#pragma once

#include <optional>

#include <imgui.h>

#include "../core/editor_events.hpp"
#include "component_inspector.hpp"
#include "panel.hpp"
#include "star/core/logger.hpp"
#include "star/ecs/component_registry.hpp"

namespace star::editor {
    class InspectorPanel final : public Panel {
      public:
        explicit InspectorPanel(ecs::ComponentRegistry* registry, ComponentInspector* inspector)
            : Panel("Inspector"), m_registry(registry), m_inspector(inspector) {}

        void on_attach() override {
            EditorEventBus::instance().subscribe(EditorEventType::EntitySelected, [this](const EditorEvent& event) {
                if (const auto* data = event.get_data<NodeSelectedEvent>())
                    m_selected_entity = data->node;
            });
        }

        void on_imgui_render() override {
            if (!m_is_open)
                return;

            ImGui::Begin(m_name.c_str(), &m_is_open);

            if (has_valid_entity())
                render_entity_contents();
            else
                render_empty_state();

            ImGui::End();
        }

      private:
        [[nodiscard]] bool has_valid_entity() const {
            return m_selected_entity.has_value() && m_selected_entity.value();
        }

        void render_entity_contents() const {
            const auto entity = *m_selected_entity;
            render_entity_header(entity);

            ImGui::Separator();
            ImGui::Spacing();

            m_inspector->draw_components(entity, *m_registry);

            ImGui::Spacing();

            m_inspector->draw_add_popup(entity, *m_registry);
        }

        static void render_entity_header(const flecs::entity entity) {
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

        static void render_empty_state() {
            const float w = ImGui::GetWindowWidth();
            const float h = ImGui::GetWindowHeight();
            constexpr auto msg = "No entity selected";
            const float tw = ImGui::CalcTextSize(msg).x;
            ImGui::SetCursorPos({(w - tw) * 0.5f, h * 0.5f});
            ImGui::TextDisabled("%s", msg);
        }

        ecs::ComponentRegistry* m_registry{};
        ComponentInspector* m_inspector{};
        std::optional<flecs::entity> m_selected_entity;
    };

} // namespace star::editor
