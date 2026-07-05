#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "star/ecs/components/camera.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/scene/scene.hpp"
#include "star/scene/scene_serializer.hpp"

using namespace star;

TEST_CASE("serialize_scene / load_scene round-trips entities, transforms and tags", "[scene][serialize]") {
    scene::Scene scene("SerializerTestScene");

    scene.create_entity("Cube")
        .set<components::Transform>({.position = Vector3{1.0f, 2.0f, 3.0f}})
        .add<components::PrimaryCamera>();
    scene.create_entity("Sphere").set<components::Transform>({.position = Vector3{-5.0f, 0.0f, 0.0f}});

    const nlohmann::json snapshot = scene::serialize_scene(scene);

    scene.find_entity("Cube").get_mut<components::Transform>()->position = Vector3{99.0f, 99.0f, 99.0f};

    scene::load_scene(scene, snapshot);

    const auto cube = scene.find_entity("Cube");
    REQUIRE(cube.is_valid());
    REQUIRE(cube.try_get<components::Transform>()->position.x == Catch::Approx(1.0f));
    REQUIRE(cube.try_get<components::Transform>()->position.y == Catch::Approx(2.0f));
    REQUIRE(cube.has<components::PrimaryCamera>());

    const auto sphere = scene.find_entity("Sphere");
    REQUIRE(sphere.is_valid());
    REQUIRE(sphere.try_get<components::Transform>()->position.x == Catch::Approx(-5.0f));
}

TEST_CASE("serialize_scene preserves parent/child hierarchy", "[scene][serialize]") {
    scene::Scene scene("Tree");

    auto parent = scene.create_entity("Parent");
    parent.set<components::Transform>({.position = Vector3{5.0f, 0.0f, 0.0f}});
    auto child = scene.create_entity("Child");
    child.set<components::Transform>({.position = Vector3{1.0f, 0.0f, 0.0f}});
    child.child_of(parent);

    const nlohmann::json document = scene::serialize_scene(scene);

    REQUIRE(document["entities"].size() == 1);
    REQUIRE(document["entities"][0]["name"] == "Parent");
    REQUIRE(document["entities"][0].contains("children"));
    REQUIRE(document["entities"][0]["children"].size() == 1);
    CHECK(document["entities"][0]["children"][0]["name"] == "Child");
    
    scene::Scene reloaded("TreeReloaded");
    scene::load_scene(reloaded, document);
    const nlohmann::json round_tripped = scene::serialize_scene(reloaded);

    CHECK(round_tripped["entities"] == document["entities"]);
}
