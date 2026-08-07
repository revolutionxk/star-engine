#pragma once

#include <flecs.h>

#include "../core/commands/command_stack.hpp"
#include "components/ui_components.hpp"
#include "star/ecs/component_registry.hpp"
#include "star/rendering/components/material_instance.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/resources/resource_manager.hpp"

namespace star::editor {
    class IconRegistry;

    class ComponentInspector {
      public:
        void set_resource_manager(resources::ResourceManager* rm) {
            m_resource_manager = rm;
        }

        void set_icons(IconRegistry* icons) {
            m_icons = icons;
        }

        void set_command_stack(CommandStack* stack) noexcept {
            m_command_stack = stack;
        }

        [[nodiscard]] resources::ResourceManager* resource_manager() const {
            return m_resource_manager;
        }

        void draw_components(flecs::entity entity) const;
        void draw_add_popup(flecs::entity entity) const;

      private:
        [[nodiscard]] const resources::Material* resolve_material(flecs::entity entity,
                                                                  const components::MaterialInstance& inst) const;

        resources::ResourceManager* m_resource_manager = nullptr;
        IconRegistry* m_icons = nullptr;
        CommandStack* m_command_stack{nullptr};
    };
} // namespace star::editor
