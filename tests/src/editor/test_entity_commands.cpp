#include <catch2/catch_test_macros.hpp>

#include "core/commands/entity_commands.hpp"
#include "star/ecs/components/transform.hpp"

using namespace star;
using namespace star::editor;

TEST_CASE("SetComponentCommand applies and reverts a component", "[editor][commands]") {
    flecs::world world;
    ecs::register_component<components::Transform>();

    auto entity = world.entity("Subject");
    entity.set<components::Transform>({});

    components::Transform before;
    before.position = {0.0f, 0.0f, 0.0f};
    components::Transform after;
    after.position = {7.0f, 8.0f, 9.0f};

    SetComponentCommand command{entity, std::type_index{typeid(components::Transform)}, std::any{before},
                                std::any{after}, "Move"};

    command.execute();
    REQUIRE(entity.get<components::Transform>().position.x == 7.0f);

    command.undo();
    REQUIRE(entity.get<components::Transform>().position.x == 0.0f);

    REQUIRE(command.label() == "Move");
    REQUIRE(command.is_valid());
}

TEST_CASE("SetComponentCommand is invalid once the entity dies", "[editor][commands]") {
    flecs::world world;
    ecs::register_component<components::Transform>();

    auto entity = world.entity();
    entity.set<components::Transform>({});

    SetComponentCommand command{entity, std::type_index{typeid(components::Transform)},
                                std::any{components::Transform{}}, std::any{components::Transform{}}, "Move"};

    entity.destruct();
    REQUIRE_FALSE(command.is_valid());
}

TEST_CASE("SetComponentCommand adds the component when the entity lacks it", "[editor][commands]") {
    flecs::world world;
    ecs::register_component<components::Transform>();

    auto entity = world.entity();

    components::Transform before;
    components::Transform after;
    after.position = {1.0f, 2.0f, 3.0f};

    SetComponentCommand command{entity, std::type_index{typeid(components::Transform)}, std::any{before},
                                std::any{after}, "Add"};

    command.execute();

    REQUIRE(entity.has<components::Transform>());
    REQUIRE(entity.get<components::Transform>().position.x == 1.0f);
    REQUIRE(entity.get<components::Transform>().position.y == 2.0f);
    REQUIRE(entity.get<components::Transform>().position.z == 3.0f);
}

TEST_CASE("clone_component returns an empty box for a missing component", "[editor][commands]") {
    flecs::world world;
    ecs::register_component<components::Transform>();

    const auto entity = world.entity();
    const std::any boxed = clone_component(entity, std::type_index{typeid(components::Transform)});
    REQUIRE_FALSE(boxed.has_value());
}
