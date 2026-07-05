#pragma once

#include "star/ecs/world.hpp"
#include "star/math/vector3.hpp"
#include "star/physics/physics_world.hpp"

namespace star::physics {
    class PhysicsSystem {
      public:
        explicit PhysicsSystem(const Vector3& gravity = {0.0f, -9.81f, 0.0f});

        void update(ecs::World& world, f32 fixed_dt);

      private:
        PhysicsWorld m_world;
    };
} // namespace star::physics
