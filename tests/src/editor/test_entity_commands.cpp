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

TEST_CASE("AddComponentCommand adds and removes", "[editor][commands]") {
    flecs::world world;
    ecs::register_component<components::Transform>();

    auto entity = world.entity();
    const std::type_index type{typeid(components::Transform)};

    AddComponentCommand command{entity, type, "Add Transform"};

    command.execute();
    REQUIRE(entity.has<components::Transform>());

    command.undo();
    REQUIRE_FALSE(entity.has<components::Transform>());
}

TEST_CASE("RemoveComponentCommand restores the component values", "[editor][commands]") {
    flecs::world world;
    ecs::register_component<components::Transform>();

    auto entity = world.entity();
    components::Transform transform;
    transform.position = {3.0f, 0.0f, 0.0f};
    entity.set<components::Transform>(transform);

    const std::type_index type{typeid(components::Transform)};
    RemoveComponentCommand command{entity, type, "Remove Transform"};

    command.execute();
    REQUIRE_FALSE(entity.has<components::Transform>());

    command.undo();
    REQUIRE(entity.has<components::Transform>());
    REQUIRE(entity.get<components::Transform>().position.x == 3.0f);
}

TEST_CASE("flecs revives an id after it was destroyed", "[editor][commands]") {
    flecs::world world;

    const auto churn = world.entity();
    const flecs::entity_t churn_id = churn.id();
    churn.destruct();

    const auto entity = world.entity("Subject");
    const flecs::entity_t id = entity.id();
    REQUIRE(ecs_strip_generation(id) == ecs_strip_generation(churn_id));
    REQUIRE(id != ecs_strip_generation(id));

    entity.mut(world).destruct();
    REQUIRE_FALSE(world.entity(id).is_alive());

    const auto revived = world.make_alive(id);
    REQUIRE(revived.is_alive());
    REQUIRE(revived.id() == id);
}

TEST_CASE("DestroyEntityCommand restores id, name, parent and components", "[editor][commands]") {
    flecs::world world;
    ecs::register_component<components::Transform>();

    const auto parent = world.entity("Parent");

    const auto churn = world.entity();
    churn.destruct();

    auto entity = world.entity("Child");
    entity.child_of(parent);
    components::Transform transform;
    transform.position = {5.0f, 0.0f, 0.0f};
    entity.set<components::Transform>(transform);

    const flecs::entity_t id = entity.id();
    REQUIRE(id != ecs_strip_generation(id));
    DestroyEntityCommand command{entity, "Delete Child"};

    command.execute();
    REQUIRE_FALSE(world.entity(id).is_alive());

    command.undo();
    const auto restored = world.entity(id);
    REQUIRE(restored.is_alive());
    REQUIRE(restored.id() == id);
    REQUIRE(std::string_view{restored.name().c_str()} == "Child");
    REQUIRE(restored.parent() == parent);
    REQUIRE(restored.get<components::Transform>().position.x == 5.0f);
}

TEST_CASE("CreateEntityCommand creates and removes the same id", "[editor][commands]") {
    flecs::world world;

    const auto parent = world.entity("Parent");
    CreateEntityCommand command{world, "Fresh", parent, "Create Entity"};

    command.execute();
    const flecs::entity_t id = command.created_id();
    REQUIRE(world.entity(id).is_alive());

    command.undo();
    REQUIRE_FALSE(world.entity(id).is_alive());

    command.execute();
    REQUIRE(world.entity(id).is_alive());
    REQUIRE(command.created_id() == id);
}

TEST_CASE("ReparentCommand moves the entity and puts it back", "[editor][commands]") {
    flecs::world world;

    const auto first = world.entity("First");
    const auto second = world.entity("Second");
    auto entity = world.entity("Child");
    entity.child_of(first);

    ReparentCommand command{entity, second, "Reparent"};

    command.execute();
    REQUIRE(entity.parent() == second);

    command.undo();
    REQUIRE(entity.parent() == first);
}

TEST_CASE("RenameEntityCommand renames and reverts", "[editor][commands]") {
    flecs::world world;
    auto entity = world.entity("Before");

    RenameEntityCommand command{entity, "After", "Rename"};

    command.execute();
    REQUIRE(std::string_view{entity.name().c_str()} == "After");

    command.undo();
    REQUIRE(std::string_view{entity.name().c_str()} == "Before");
}
