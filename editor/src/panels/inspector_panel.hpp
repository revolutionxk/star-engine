#pragma once

#include <optional>

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

      private:
        [[nodiscard]] bool has_valid_entity() const;
        void render_entity_contents() const;
        static void render_entity_header(flecs::entity entity);
        static void render_empty_state();

        ComponentInspector* m_inspector{};
        std::optional<flecs::entity> m_selected_entity;
    };
} // namespace star::editor
