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
