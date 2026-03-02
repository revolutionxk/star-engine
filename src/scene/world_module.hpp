#pragma once
#include "star/ecs/component_registry.hpp"
#include "star/ecs/components/tag.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/rendering/components/material_instance.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/scene/components/camera.hpp"

namespace star::scene {
    struct WorldModule {
        explicit WorldModule(flecs::world& world) {
            world.module<WorldModule>();
            world.set<flecs::Rest>({});
            ecs::register_world_components(world);
        }
    };
} // namespace star::scene
