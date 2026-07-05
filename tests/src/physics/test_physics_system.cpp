#include <catch2/catch_test_macros.hpp>

#include "star/ecs/components/transform.hpp"
#include "star/ecs/world.hpp"
#include "star/physics/components/collider.hpp"
#include "star/physics/components/rigid_body.hpp"
#include "star/physics/physics_system.hpp"

using namespace star;

TEST_CASE("A dynamic body falls under gravity and syncs back to its Transform", "[physics][system]") {
    ecs::World world;
    physics::PhysicsSystem physics;

    const auto entity = world.native()
                            .entity()
                            .set<components::Transform>({.position = {0.0f, 10.0f, 0.0f}})
                            .set<components::RigidBody>({.motion = physics::MotionType::Dynamic})
                            .set<components::Collider>({.shape = physics::ColliderShape::Box});

    const f32 start_y = entity.try_get<components::Transform>()->position.y;

    for (int i = 0; i < 60; ++i) {
        physics.update(world, 1.0f / 60.0f);
    }

    const f32 end_y = entity.try_get<components::Transform>()->position.y;
    REQUIRE(end_y < start_y - 1.0f);
}

TEST_CASE("A static body stays put", "[physics][system]") {
    ecs::World world;
    physics::PhysicsSystem physics;

    const auto entity = world.native()
                            .entity()
                            .set<components::Transform>({.position = {0.0f, 3.0f, 0.0f}})
                            .set<components::RigidBody>({.motion = physics::MotionType::Static})
                            .set<components::Collider>({.shape = physics::ColliderShape::Box});

    for (int i = 0; i < 30; ++i) {
        physics.update(world, 1.0f / 60.0f);
    }

    REQUIRE(entity.try_get<components::Transform>()->position.y == 3.0f);
}
