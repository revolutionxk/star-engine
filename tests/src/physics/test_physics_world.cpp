#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "star/physics/physics_world.hpp"

using namespace star;
using namespace star::physics;

TEST_CASE("PhysicsWorld initializes Jolt and steps an empty world", "[physics][world]") {
    PhysicsWorld world;
    for (int i = 0; i < 10; ++i) {
        world.step(1.0f / 60.0f);
    }
    SUCCEED("stepped without a crash");
}

TEST_CASE("PhysicsWorld can be created and torn down repeatedly", "[physics][world]") {
    for (int i = 0; i < 3; ++i) {
        PhysicsWorld world;
        world.step(1.0f / 60.0f);
    }
    SUCCEED("Jolt global init/shutdown is reference counted correctly");
}

TEST_CASE("apply_impulse gives a dynamic body upward velocity", "[physics][world]") {
    const PhysicsWorld world;

    BodyDesc desc;
    desc.motion = MotionType::Dynamic;
    desc.position = {0.0f, 5.0f, 0.0f};
    const BodyHandle body = world.create_body(desc);

    world.apply_impulse(body, {0.0f, 5000.0f, 0.0f});

    REQUIRE(world.linear_velocity(body).y > 0.0f);
}

TEST_CASE("set_linear_velocity round-trips through linear_velocity", "[physics][world]") {
    PhysicsWorld world;

    BodyDesc desc;
    desc.motion = MotionType::Dynamic;
    desc.position = {0.0f, 5.0f, 0.0f};
    const BodyHandle body = world.create_body(desc);

    world.set_linear_velocity(body, {3.0f, 0.0f, -2.0f});

    const Vector3 velocity = world.linear_velocity(body);
    REQUIRE(velocity.x == Catch::Approx(3.0f).margin(0.01f));
    REQUIRE(velocity.z == Catch::Approx(-2.0f).margin(0.01f));
}

TEST_CASE("a capsule body is created and falls", "[physics][world]") {
    PhysicsWorld world;

    BodyDesc desc;
    desc.motion = MotionType::Dynamic;
    desc.shape = ColliderShape::Capsule;
    desc.radius = 0.3f;
    desc.half_height = 0.6f;
    desc.position = {0.0f, 10.0f, 0.0f};
    const BodyHandle body = world.create_body(desc);

    Vector3 start;
    Quaternion rotation;
    world.body_transform(body, start, rotation);

    for (int i = 0; i < 30; ++i) {
        world.step(1.0f / 60.0f);
    }

    Vector3 end;
    world.body_transform(body, end, rotation);
    REQUIRE(end.y < start.y);
}
