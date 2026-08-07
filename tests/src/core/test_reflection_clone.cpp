#include <any>

#include <catch2/catch_test_macros.hpp>

#include "star/core/reflection/type_registry.hpp"
#include "star/ecs/components/transform.hpp"

using namespace star;

TEST_CASE("clone and restore round-trip a component", "[core][reflection]") {
    auto& registry = reflection::TypeRegistry::instance();
    registry.register_type<components::Transform>();

    const auto info = registry.find<components::Transform>();
    REQUIRE(info.has_value());
    REQUIRE(static_cast<bool>((*info)->clone));
    REQUIRE(static_cast<bool>((*info)->restore));

    components::Transform source;
    source.position = {1.0f, 2.0f, 3.0f};
    source.scale = {4.0f, 5.0f, 6.0f};

    const std::any boxed = (*info)->clone(&source);

    components::Transform target;
    REQUIRE((*info)->restore(&target, boxed));
    REQUIRE(target.position.x == 1.0f);
    REQUIRE(target.position.y == 2.0f);
    REQUIRE(target.position.z == 3.0f);
    REQUIRE(target.scale.x == 4.0f);
}

TEST_CASE("restore rejects a box of the wrong type", "[core][reflection]") {
    auto& registry = reflection::TypeRegistry::instance();
    registry.register_type<components::Transform>();

    const auto info = registry.find<components::Transform>();
    REQUIRE(info.has_value());

    components::Transform target;
    const std::any wrong = 42;
    REQUIRE_FALSE((*info)->restore(&target, wrong));
}
