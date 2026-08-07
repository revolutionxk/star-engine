#include <memory>

#include <catch2/catch_test_macros.hpp>

#include "core/commands/command_stack.hpp"
#include "core/commands/entity_commands.hpp"
#include "star/ecs/components/camera.hpp"
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

TEST_CASE("DestroyEntityCommand restores an entire subtree", "[editor][commands]") {
    flecs::world world;
    ecs::register_component<components::Transform>();

    const auto root = world.entity("Root");

    auto parent = world.entity("Parent");
    parent.child_of(root);

    auto child_a = world.entity("ChildA");
    child_a.child_of(parent);
    components::Transform child_transform;
    child_transform.position = {1.0f, 2.0f, 3.0f};
    child_a.set<components::Transform>(child_transform);

    auto child_b = world.entity("ChildB");
    child_b.child_of(parent);

    auto grandchild = world.entity("Grandchild");
    grandchild.child_of(child_a);
    components::Transform grandchild_transform;
    grandchild_transform.position = {4.0f, 5.0f, 6.0f};
    grandchild.set<components::Transform>(grandchild_transform);

    const flecs::entity_t parent_id = parent.id();
    const flecs::entity_t child_a_id = child_a.id();
    const flecs::entity_t child_b_id = child_b.id();
    const flecs::entity_t grandchild_id = grandchild.id();

    DestroyEntityCommand command{parent, "Delete Parent"};

    command.execute();
    REQUIRE_FALSE(world.entity(parent_id).is_alive());
    REQUIRE_FALSE(world.entity(child_a_id).is_alive());
    REQUIRE_FALSE(world.entity(child_b_id).is_alive());
    REQUIRE_FALSE(world.entity(grandchild_id).is_alive());

    REQUIRE(command.is_valid());
    command.undo();

    const auto restored_parent = world.entity(parent_id);
    const auto restored_a = world.entity(child_a_id);
    const auto restored_b = world.entity(child_b_id);
    const auto restored_grandchild = world.entity(grandchild_id);

    REQUIRE(restored_parent.is_alive());
    REQUIRE(restored_a.is_alive());
    REQUIRE(restored_b.is_alive());
    REQUIRE(restored_grandchild.is_alive());

    REQUIRE(std::string_view{restored_parent.name().c_str()} == "Parent");
    REQUIRE(std::string_view{restored_a.name().c_str()} == "ChildA");
    REQUIRE(std::string_view{restored_b.name().c_str()} == "ChildB");
    REQUIRE(std::string_view{restored_grandchild.name().c_str()} == "Grandchild");

    REQUIRE(restored_parent.parent() == root);
    REQUIRE(restored_a.parent() == restored_parent);
    REQUIRE(restored_b.parent() == restored_parent);
    REQUIRE(restored_grandchild.parent() == restored_a);

    REQUIRE(restored_a.get<components::Transform>().position.x == 1.0f);
    REQUIRE(restored_a.get<components::Transform>().position.y == 2.0f);
    REQUIRE(restored_grandchild.get<components::Transform>().position.x == 4.0f);
    REQUIRE(restored_grandchild.get<components::Transform>().position.z == 6.0f);
}

TEST_CASE("DestroyEntityCommand refuses to revive a reused index", "[editor][commands]") {
    flecs::world world;
    ecs::register_component<components::Transform>();

    auto entity = world.entity("Subject");
    entity.set<components::Transform>({});
    const flecs::entity_t id = entity.id();

    DestroyEntityCommand command{entity, "Delete Subject"};
    command.execute();
    REQUIRE_FALSE(world.entity(id).is_alive());

    const auto squatter = world.entity("Squatter");
    REQUIRE(ecs_strip_generation(squatter.id()) == ecs_strip_generation(id));
    REQUIRE(squatter.id() != id);

    REQUIRE_FALSE(command.is_valid());

    command.undo();
    REQUIRE(world.entity(squatter.id()).is_alive());
    REQUIRE(std::string_view{squatter.name().c_str()} == "Squatter");
    REQUIRE_FALSE(world.entity(id).is_alive());
}

TEST_CASE("CommandStack clears rather than reviving a reused index", "[editor][commands]") {
    flecs::world world;
    ecs::register_component<components::Transform>();

    auto entity = world.entity("Subject");
    entity.set<components::Transform>({});

    CommandStack stack;
    stack.push(std::make_unique<DestroyEntityCommand>(entity, "Delete Subject"));
    REQUIRE(stack.can_undo());

    const auto squatter = world.entity("Squatter");
    REQUIRE(squatter.is_alive());

    stack.undo();
    REQUIRE_FALSE(stack.can_undo());
    REQUIRE_FALSE(stack.can_redo());
    REQUIRE(squatter.is_alive());
}

TEST_CASE("DestroyEntityCommand reports invalid when the recorded parent has died", "[editor][commands]") {
    flecs::world world;
    ecs::register_component<components::Transform>();

    auto parent = world.entity("Parent");
    auto child = world.entity("Child");
    child.child_of(parent);
    child.set<components::Transform>({});

    const flecs::entity_t child_id = child.id();

    DestroyEntityCommand command{child, "Delete Child"};
    command.execute();
    REQUIRE_FALSE(world.entity(child_id).is_alive());

    parent.destruct();

    REQUIRE_FALSE(command.is_valid());
    command.undo();

    const auto restored = world.entity(child_id);
    REQUIRE(restored.is_alive());
    REQUIRE(restored.parent() == 0);
}

TEST_CASE("DestroyEntityCommand restores tag components", "[editor][commands]") {
    flecs::world world;
    ecs::register_component<components::Transform>();
    ecs::register_component<components::PrimaryCamera>(ecs::RegistrationFlags::Hidden);

    auto entity = world.entity("Camera");
    entity.set<components::Transform>({});
    entity.add<components::PrimaryCamera>();

    const flecs::entity_t id = entity.id();
    DestroyEntityCommand command{entity, "Delete Camera"};

    command.execute();
    REQUIRE_FALSE(world.entity(id).is_alive());

    command.undo();
    const auto restored = world.entity(id);
    REQUIRE(restored.is_alive());
    REQUIRE(restored.has<components::PrimaryCamera>());
    REQUIRE(restored.has<components::Transform>());
}

TEST_CASE("RemoveComponentCommand restores a tag without wiping the stack", "[editor][commands]") {
    flecs::world world;
    ecs::register_component<components::PrimaryCamera>(ecs::RegistrationFlags::Hidden);

    auto entity = world.entity("Camera");
    entity.add<components::PrimaryCamera>();

    const std::type_index type{typeid(components::PrimaryCamera)};
    CommandStack stack;
    stack.push(std::make_unique<RemoveComponentCommand>(entity, type, "PrimaryCamera"));
    REQUIRE_FALSE(entity.has<components::PrimaryCamera>());

    stack.undo();
    REQUIRE(entity.has<components::PrimaryCamera>());
    REQUIRE(stack.can_redo());
    REQUIRE_FALSE(stack.can_undo());

    stack.redo();
    REQUIRE_FALSE(entity.has<components::PrimaryCamera>());
    REQUIRE(stack.can_undo());
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

TEST_CASE("CreateEntityCommand uniquifies a name already used in the scope", "[editor][commands]") {
    flecs::world world;

    const auto parent = world.entity("Parent");

    CreateEntityCommand first{world, "Camera", parent, "Create Camera"};
    CreateEntityCommand second{world, "Camera", parent, "Create Camera"};

    first.execute();
    second.execute();

    REQUIRE(first.created_id() != second.created_id());
    REQUIRE(std::string_view{world.entity(first.created_id()).name().c_str()} == "Camera");
    REQUIRE(std::string_view{world.entity(second.created_id()).name().c_str()} == "Camera (1)");
}

TEST_CASE("CreateEntityCommand never adopts a pre-existing entity", "[editor][commands]") {
    flecs::world world;

    const auto parent = world.entity("Parent");
    const auto existing = world.entity("Camera");
    existing.child_of(parent);
    const flecs::entity_t existing_id = existing.id();

    CreateEntityCommand command{world, "Camera", parent, "Create Camera"};
    command.execute();
    REQUIRE(command.created_id() != existing_id);

    command.undo();
    REQUIRE(world.entity(existing_id).is_alive());
    REQUIRE(std::string_view{world.entity(existing_id).name().c_str()} == "Camera");
    REQUIRE_FALSE(world.entity(command.created_id()).is_alive());
}

TEST_CASE("CreateEntityCommand is invalid once its index is reused", "[editor][commands]") {
    flecs::world world;

    const auto parent = world.entity("Parent");
    CreateEntityCommand command{world, "Fresh", parent, "Create Entity"};

    command.execute();
    const flecs::entity_t id = command.created_id();
    REQUIRE(command.is_valid());

    command.undo();
    const auto squatter = world.entity("Squatter");
    REQUIRE(ecs_strip_generation(squatter.id()) == ecs_strip_generation(id));

    REQUIRE_FALSE(command.is_valid());

    command.execute();
    REQUIRE(world.entity(squatter.id()).is_alive());
    REQUIRE(std::string_view{squatter.name().c_str()} == "Squatter");
    REQUIRE(command.created_id() != squatter.id());
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

TEST_CASE("ReparentCommand moves an entity to no parent and back", "[editor][commands]") {
    flecs::world world;

    const auto first = world.entity("First");
    auto entity = world.entity("Child");
    entity.child_of(first);

    ReparentCommand command{entity, flecs::entity{}, "Reparent"};

    command.execute();
    REQUIRE_FALSE(entity.parent());

    command.undo();
    REQUIRE(entity.parent() == first);
}

TEST_CASE("ReparentCommand adopts a root entity and releases it", "[editor][commands]") {
    flecs::world world;

    const auto first = world.entity("First");
    auto entity = world.entity("Child");

    ReparentCommand command{entity, first, "Reparent"};

    command.execute();
    REQUIRE(entity.parent() == first);

    command.undo();
    REQUIRE_FALSE(entity.parent());
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
