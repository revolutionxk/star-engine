#pragma once

#include <array>

#include <imgui.h>

#include "../core/editor_events.hpp"
#include "panel.hpp"
#include "star/core/logger.hpp"
#include "star/ecs/components/transform.hpp"

namespace star::editor {

    class InspectorPanel final : public Panel {
      public:
        InspectorPanel() : Panel("Inspector") {}

        void on_attach() override {
            EditorEventBus::instance().subscribe(EditorEventType::EntitySelected, [this](const EditorEvent& event) {
                if (const auto* data = event.get_data<NodeSelectedEvent>()) {
                    set_selected_entity(data->node);
                }
            });
        }

        void set_selected_entity(flecs::entity& entity) {
            m_selected_entity = entity;
            if (m_selected_entity.has_value() && m_selected_entity.value()) {
                auto transform = m_selected_entity->try_get<components::Transform>();
                if (!transform)
                    return;

                auto& [position, rotation, scale] = *transform;
                m_transform_position = position;
                m_transform_rotation = rotation.to_euler() * (180.0f / math::Constants<f32>::pi);
                m_transform_scale = scale;
            }
        }

        void on_imgui_render() override {
            if (!m_is_open)
                return;

            ImGui::Begin(m_name.c_str(), &m_is_open);

            if (has_valid_entity()) {
                render_entity_info();
            } else {
                render_empty_state();
            }

            ImGui::End();
        }

      private:
        [[nodiscard]] bool has_valid_entity() const {
            if (!m_selected_entity)
                return false;

            if (!m_selected_entity.has_value())
                return false;

            return m_selected_entity.value();
        }

        void render_entity_info() {
            render_entity_name();
            ImGui::Separator();
            ImGui::Spacing();

            render_transform_component();

            ImGui::Spacing();
            render_add_component_button();
        }

        void render_entity_name() {
            if (!m_selected_entity)
                return;
            ImGui::Text("Entity Name");
            ImGui::SameLine();

            std::strncpy(m_name_buffer.data(), m_selected_entity->name().c_str(), m_name_buffer.size() - 1);

            if (ImGui::InputText("##EntityName", m_name_buffer.data(), m_name_buffer.size())) {
                // Rename entity logic
            }
        }

        void render_transform_component() {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::PushItemWidth(-120);

                if (ImGui::DragFloat3("Position", m_transform_position.data, 0.1f)) {
                    if (m_selected_entity.has_value()) {
                        auto& transform = m_selected_entity->get<components::Transform>();
                        transform.position = m_transform_position;
                    }
                }

                if (ImGui::DragFloat3("Rotation", m_transform_rotation.data, 0.5f)) {
                    if (m_selected_entity.has_value()) {
                        auto& transform = m_selected_entity->get<components::Transform>();
                        transform.rotation =
                            Quaternion::from_euler(radians(m_transform_rotation.x), radians(m_transform_rotation.y),
                                                   radians(m_transform_rotation.z));
                    }
                }

                if (ImGui::DragFloat3("Scale", m_transform_scale.data, 0.05f, 0.01f, 100.0f)) {
                    if (m_selected_entity.has_value()) {
                        auto& transform = m_selected_entity->get<components::Transform>();
                        transform.scale = m_transform_scale;
                    }
                }

                ImGui::PopItemWidth();
            }
        }

        void render_add_component_button() {
            if (ImGui::Button("Add Component", ImVec2(-1, 0))) {
                ImGui::OpenPopup("AddComponentPopup");
            }

            if (ImGui::BeginPopup("AddComponentPopup")) {
                ImGui::Text("Available Components");
                ImGui::Separator();

                if (ImGui::Selectable("Mesh Renderer")) {
                    add_component("MeshRenderer");
                }
                if (ImGui::Selectable("Camera")) {
                    add_component("Camera");
                }
                if (ImGui::Selectable("Light")) {
                    add_component("Light");
                }

                ImGui::EndPopup();
            }
        }

        static void render_empty_state() {
            const float window_width = ImGui::GetWindowWidth();
            const auto message = "No entity selected";
            const float text_width = ImGui::CalcTextSize(message).x;

            ImGui::SetCursorPosX((window_width - text_width) * 0.5f);
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.5f);
            ImGui::TextDisabled("%s", message);
        }

        void add_component(const char* component_name) {
            STAR_LOG_INFO(LogCategory::Editor, "Adding component: {}", component_name);
            // Component addition logic
        }

        std::optional<flecs::entity> m_selected_entity;
        std::array<char, 256> m_name_buffer{};
        Vector3 m_transform_position{0.0f, 0.0f, 0.0f};
        Vector3 m_transform_rotation{0.0f, 0.0f, 0.0f};
        Vector3 m_transform_scale{1.0f, 1.0f, 1.0f};
    };

} // namespace star::editor
