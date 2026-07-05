#pragma once

#include <unordered_map>

#include "star/ecs/world.hpp"
#include "star/math/math.hpp"
#include "star/physics/physics_world.hpp"

namespace star::physics {
    class PhysicsSystem {
      public:
        explicit PhysicsSystem(const Vector3& gravity = {0.0f, -9.81f, 0.0f});

        void update(ecs::World& world, f32 fixed_dt);
        void reset(ecs::World& world);

        void apply_impulse(BodyHandle body, const Vector3& impulse) const;
        void set_linear_velocity(BodyHandle body, const Vector3& velocity) const;
        [[nodiscard]] Vector3 linear_velocity(BodyHandle body) const;

      private:
        struct BodyState {
            Vector3 position{};
            Quaternion rotation{0.0f, 0.0f, 0.0f, 1.0f};
        };

        PhysicsWorld m_world;
        std::unordered_map<BodyHandle, BodyState> m_body_states;
    };
} // namespace star::physics
