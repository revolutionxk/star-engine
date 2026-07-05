#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "star/ecs/components/transform.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/rendering/render_scene.hpp"
#include "star/scene/scene.hpp"
#include "star/scene/scene_render_extractor.hpp"

using namespace star;

TEST_CASE("SceneRenderExtractor composes parent world transform into children", "[scene][hierarchy]") {
    scene::Scene scene("Hierarchy");

    constexpr graphics::ResourceHandle<resources::Mesh> mesh{1, 1};

    auto parent = scene.create_entity("Parent");
    parent.set<components::Transform>({.position = Vector3{10.0f, 0.0f, 0.0f}})
        .set<components::MeshRenderer>({.mesh = mesh});

    auto child = scene.create_entity("Child");
    child.set<components::Transform>({.position = Vector3{1.0f, 0.0f, 0.0f}})
        .set<components::MeshRenderer>({.mesh = mesh});
    child.child_of(parent);

    scene::SceneRenderExtractor extractor;
    rendering::RenderScene render_scene;
    extractor.extract(scene, render_scene);

    REQUIRE(render_scene.renderables.size() == 2);

    bool child_is_at_world_11 = false;
    bool parent_is_at_world_10 = false;
    for (const auto& r : render_scene.renderables) {
        if (r.world_position.x == Catch::Approx(11.0f))
            child_is_at_world_11 = true;
        if (r.world_position.x == Catch::Approx(10.0f))
            parent_is_at_world_10 = true;
    }
    CHECK(parent_is_at_world_10);
    CHECK(child_is_at_world_11);
}

TEST_CASE("SceneRenderExtractor leaves unparented entities at their local transform", "[scene][hierarchy]") {
    scene::Scene scene("Flat");
    constexpr graphics::ResourceHandle<resources::Mesh> mesh{1, 1};

    scene.create_entity("Solo")
        .set<components::Transform>({.position = Vector3{5.0f, 2.0f, -3.0f}})
        .set<components::MeshRenderer>({.mesh = mesh});

    scene::SceneRenderExtractor extractor;
    rendering::RenderScene render_scene;
    extractor.extract(scene, render_scene);

    REQUIRE(render_scene.renderables.size() == 1);
    CHECK(render_scene.renderables[0].world_position.x == Catch::Approx(5.0f));
    CHECK(render_scene.renderables[0].world_position.y == Catch::Approx(2.0f));
    CHECK(render_scene.renderables[0].world_position.z == Catch::Approx(-3.0f));
}
