#include "star/physics/physics_system.hpp"

#include "star/ecs/components/transform.hpp"
#include "star/physics/components/collider.hpp"
#include "star/physics/components/rigid_body.hpp"

namespace star::physics {
    PhysicsSystem::PhysicsSystem(const Vector3& gravity) : m_world(gravity) {}

    void PhysicsSystem::update(ecs::World& world, const f32 fixed_dt) {
        const auto& flecs_world = world.native();

        flecs_world.each([this](const components::Transform& transform, components::RigidBody& rigid_body,
                                const components::Collider& collider) {
            if (rigid_body.body != INVALID_BODY)
                return;

            BodyDesc desc;
            desc.shape = collider.shape;
            desc.half_extents = collider.half_extents;
            desc.radius = collider.radius;
            desc.position = transform.position;
            desc.rotation = transform.rotation;
            desc.motion = rigid_body.motion;
            desc.friction = rigid_body.friction;
            desc.restitution = rigid_body.restitution;

            rigid_body.body = m_world.create_body(desc);
        });

        m_world.step(fixed_dt);

        flecs_world.each([this](components::Transform& transform, const components::RigidBody& rigid_body) {
            if (rigid_body.body == INVALID_BODY || rigid_body.motion != MotionType::Dynamic)
                return;

            Vector3 position;
            Quaternion rotation;
            if (m_world.body_transform(rigid_body.body, position, rotation)) {
                transform.position = position;
                transform.rotation = rotation;
            }
        });
    }
} // namespace star::physics
