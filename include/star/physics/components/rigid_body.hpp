#pragma once

#include "star/ecs/component_registry.hpp"
#include "star/physics/physics_world.hpp"

namespace star::components {
    struct RigidBody {
        physics::MotionType motion = physics::MotionType::Dynamic;
        f32 friction = 0.2f;
        f32 restitution = 0.0f;

        physics::BodyHandle body = physics::INVALID_BODY;
    };
} // namespace star::components

namespace star {
    template<>
    struct reflection::TypeInfo<components::RigidBody> {
        static constexpr std::string_view name = "RigidBody";
        static constexpr bool is_component = true;
        static constexpr auto fields = std::make_tuple(
            field("Motion", &components::RigidBody::motion) | attr::EnumOptions("Static", "Kinematic", "Dynamic"),
            field("Friction", &components::RigidBody::friction) | attr::Speed{0.01f} | attr::Range{0.0f, 1.0f},
            field("Restitution", &components::RigidBody::restitution) | attr::Speed{0.01f} | attr::Range{0.0f, 1.0f},
            field("Body", &components::RigidBody::body) | attr::HideInEditor{});
    };
} // namespace star

STAR_REGISTER_COMPONENT(star::components::RigidBody);
