#pragma once

#include "star/ecs/component_registry.hpp"
#include "star/math/math.hpp"
#include "star/physics/physics_world.hpp"

namespace star::components {
    struct Collider {
        physics::ColliderShape shape = physics::ColliderShape::Box;
        Vector3 half_extents{0.5f, 0.5f, 0.5f};
        f32 radius = 0.5f;
        f32 half_height = 0.5f;
    };
} // namespace star::components

namespace star {
    template<>
    struct reflection::TypeInfo<components::Collider> {
        static constexpr std::string_view name = "Collider";
        static constexpr bool is_component = true;
        static constexpr auto fields = std::make_tuple(
            field("Shape", &components::Collider::shape) | attr::EnumOptions("Box", "Sphere", "Capsule"),
            field("Half Extents", &components::Collider::half_extents) | attr::Speed{0.05f},
            field("Radius", &components::Collider::radius) | attr::Speed{0.05f} | attr::Range{0.01f, 1000.0f},
            field("Half Height", &components::Collider::half_height) | attr::Speed{0.05f} | attr::Range{0.01f, 1000.0f});
    };
} // namespace star

STAR_REGISTER_COMPONENT(star::components::Collider);
