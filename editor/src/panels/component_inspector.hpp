#pragma once

#include <flecs.h>

#include "components/ui_components.hpp"
#include "star/ecs/component_registry.hpp"
#include "star/rendering/components/material_instance.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/resources/resource_manager.hpp"

namespace star::editor {
    class ComponentInspector {
      public:
        void set_resource_manager(resources::ResourceManager* rm) {
            m_resource_manager = rm;
        }

        void draw_components(flecs::entity entity) const;
        static void draw_add_popup(flecs::entity entity);

      private:
        [[nodiscard]] const resources::Material* resolve_material(flecs::entity entity,
                                                                  const components::MaterialInstance& inst) const;

        resources::ResourceManager* m_resource_manager = nullptr;
    };
} // namespace star::editor
