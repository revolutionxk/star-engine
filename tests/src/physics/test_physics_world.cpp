#include <catch2/catch_test_macros.hpp>

#include "star/physics/physics_world.hpp"

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
