#pragma once
#include "star/ecs/components/tag.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/rendering/components/material.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/scene/components/camera.hpp"

namespace star::scene {
    struct WorldModule {
        explicit WorldModule(const flecs::world& world) {
            world.module<WorldModule>();

            world.set<flecs::Rest>({});

            world.component<components::Tag>("Tag");
            world.component<components::Transform>("Transform");
            world.component<components::Camera>("Camera");
            world.component<components::MeshRenderer>("MeshRenderer");
            world.component<components::Material>("Material");
            world.component<components::PrimaryCamera>("PrimaryCamera");
        }
    };
} // namespace star::scene
