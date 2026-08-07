#pragma once

#include <optional>
#include <string>

#include "../core/commands/command_stack.hpp"
#include "../core/editor_events.hpp"
#include "component_inspector.hpp"
#include "panel.hpp"
#include "star/ecs/component_registry.hpp"

namespace star::editor {
    class InspectorPanel final : public Panel {
      public:
        explicit InspectorPanel(ComponentInspector* inspector) : Panel("Inspector"), m_inspector(inspector) {}

        void on_attach() override;
        void on_imgui_render() override;

        void set_command_stack(CommandStack* stack) noexcept {
            m_command_stack = stack;
        }

      private:
        [[nodiscard]] bool has_valid_entity() const;
        void render_entity_contents() const;
        void render_material_editor() const;
        void render_entity_header(flecs::entity entity) const;
        static void render_empty_state();

        ComponentInspector* m_inspector{};
        CommandStack* m_command_stack{nullptr};
        std::optional<flecs::entity> m_selected_entity;
        std::optional<std::string> m_selected_material;
    };
} // namespace star::editor
