#pragma once

#include <memory>

#include "star/core/types.hpp"
#include "star/math/math.hpp"

namespace star::physics {
    enum class MotionType {
        Static,
        Kinematic,
        Dynamic,
    };

    enum class ColliderShape {
        Box,
        Sphere,
    };

    using BodyHandle = u32;
    inline constexpr BodyHandle INVALID_BODY = 0xFFFFFFFFu;

    struct BodyDesc {
        ColliderShape shape = ColliderShape::Box;
        Vector3 half_extents{0.5f, 0.5f, 0.5f};
        f32 radius = 0.5f;

        Vector3 position{};
        Quaternion rotation{0.0f, 0.0f, 0.0f, 1.0f};

        MotionType motion = MotionType::Dynamic;
        f32 friction = 0.2f;
        f32 restitution = 0.0f;
    };

    class PhysicsWorld {
      public:
        explicit PhysicsWorld(const Vector3& gravity = {0.0f, -9.81f, 0.0f});
        ~PhysicsWorld();

        PhysicsWorld(const PhysicsWorld&) = delete;
        PhysicsWorld& operator=(const PhysicsWorld&) = delete;

        void step(f32 dt);

        [[nodiscard]] BodyHandle create_body(const BodyDesc& desc) const;
        void remove_body(BodyHandle handle) const;

        [[nodiscard]] bool body_transform(BodyHandle handle, Vector3& out_position, Quaternion& out_rotation) const;
        void set_body_transform(BodyHandle handle, const Vector3& position, const Quaternion& rotation) const;

      private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
} // namespace star::physics
