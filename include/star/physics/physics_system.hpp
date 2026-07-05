#pragma once

#include "star/ecs/world.hpp"
#include "star/math/vector3.hpp"
#include "star/physics/physics_world.hpp"

namespace star::physics {
    class PhysicsSystem {
      public:
        explicit PhysicsSystem(const Vector3& gravity = {0.0f, -9.81f, 0.0f});

        void update(ecs::World& world, f32 fixed_dt);
        void reset(ecs::World& world) const;

        void apply_impulse(BodyHandle body, const Vector3& impulse) const;
        void set_linear_velocity(BodyHandle body, const Vector3& velocity) const;
        [[nodiscard]] Vector3 linear_velocity(BodyHandle body) const;

      private:
        PhysicsWorld m_world;
    };
} // namespace star::physics
