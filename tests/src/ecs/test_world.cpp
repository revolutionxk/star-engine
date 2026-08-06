#include <catch2/catch_test_macros.hpp>

#include "star/ecs/world.hpp"

using namespace star;

using namespace star::ecs;

namespace {
    struct Position {
        f32 x = 0.0f;
    };

    struct Velocity {
        f32 dx = 0.0f;
    };

    struct Counter {
        i32 ticks = 0;
    };
} // namespace

TEST_CASE("World runs a registered system every progress", "[ecs][world]") {
    World world;

    world.add_system<Position, const Velocity>("Move", Phase::OnUpdate,
                                               [](Position& p, const Velocity& v) { p.x += v.dx; });

    const auto entity = world.native().entity().set<Position>({0.0f}).set<Velocity>({5.0f});

    world.progress(1.0f);
    REQUIRE(entity.try_get<Position>()->x == 5.0f);

    world.progress(1.0f);
    REQUIRE(entity.try_get<Position>()->x == 10.0f);
}

TEST_CASE("World phases run in order within a single progress", "[ecs][world]") {
    World world;

    const auto entity = world.native().entity().set<Counter>({0});

    i32 seen_in_pre = -1;
    world.add_system<const Counter>("Observe", Phase::PreUpdate, [&](const Counter& c) { seen_in_pre = c.ticks; });
    world.add_system<Counter>("Bump", Phase::OnUpdate, [](Counter& c) { c.ticks += 1; });

    world.progress(1.0f);

    REQUIRE(seen_in_pre == 0);
    REQUIRE(entity.try_get<Counter>()->ticks == 1);
}
