#include "star/physics/physics_system.hpp"

#include "star/ecs/components/transform.hpp"
#include "star/ecs/components/transform_interpolation.hpp"
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
            desc.half_height = collider.half_height;
            desc.position = transform.position;
            desc.rotation = transform.rotation;
            desc.motion = rigid_body.motion;
            desc.friction = rigid_body.friction;
            desc.restitution = rigid_body.restitution;

            rigid_body.body = m_world.create_body(desc);
            m_body_states[rigid_body.body] = {transform.position, transform.rotation};
        });

        const auto moved_externally = [](const components::Transform& t, const BodyState& s) {
            const Vector3 dp = t.position - s.position;
            const f32 dot = t.rotation.x * s.rotation.x + t.rotation.y * s.rotation.y + t.rotation.z * s.rotation.z +
                            t.rotation.w * s.rotation.w;
            return dp.length_squared() > 1e-6f || std::abs(dot) < 0.99999f;
        };

        flecs_world.each(
            [this, &moved_externally](const components::Transform& transform, const components::RigidBody& rigid_body) {
                if (rigid_body.body == INVALID_BODY)
                    return;
                const auto it = m_body_states.find(rigid_body.body);
                if (it == m_body_states.end() || !moved_externally(transform, it->second))
                    return;

                m_world.set_body_transform(rigid_body.body, transform.position, transform.rotation);
                m_world.set_linear_velocity(rigid_body.body, Vector3{0.0f, 0.0f, 0.0f});
                it->second = {transform.position, transform.rotation};
            });

        m_world.step(fixed_dt);

        flecs_world.defer([this, &flecs_world] {
            flecs_world.each([this](const flecs::entity e, components::Transform& transform,
                                    const components::RigidBody& rigid_body) {
                if (rigid_body.body == INVALID_BODY || rigid_body.motion != MotionType::Dynamic)
                    return;

                Vector3 position;
                Quaternion rotation;
                if (m_world.body_transform(rigid_body.body, position, rotation)) {
                    e.set(components::TransformInterpolation{transform.position, transform.rotation, transform.scale});
                    transform.position = position;
                    transform.rotation = rotation;
                    m_body_states[rigid_body.body] = {position, rotation};
                }
            });
        });
    }

    void PhysicsSystem::reset(ecs::World& world) {
        auto& flecs_world = world.native();
        flecs_world.defer([this, &flecs_world] {
            flecs_world.each([this](const flecs::entity e, components::RigidBody& rigid_body) {
                if (rigid_body.body != INVALID_BODY) {
                    m_world.remove_body(rigid_body.body);
                    rigid_body.body = INVALID_BODY;
                }
                e.remove<components::TransformInterpolation>();
            });
        });
        m_body_states.clear();
    }

    void PhysicsSystem::apply_impulse(const BodyHandle body, const Vector3& impulse) const {
        m_world.apply_impulse(body, impulse);
    }

    void PhysicsSystem::set_linear_velocity(const BodyHandle body, const Vector3& velocity) const {
        m_world.set_linear_velocity(body, velocity);
    }

    Vector3 PhysicsSystem::linear_velocity(const BodyHandle body) const {
        return m_world.linear_velocity(body);
    }
} // namespace star::physics
