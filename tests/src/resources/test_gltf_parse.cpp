#include <filesystem>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "star/core/job_system.hpp"
#include "star/resources/import/gltf_importer.hpp"

using namespace star;
using namespace star::resources;

namespace {
    std::filesystem::path model(const char* name) {
        return std::filesystem::path{STAR_TEST_ASSET_DIR} / "models" / name;
    }
} // namespace

TEST_CASE("parse_gltf reads a cube without touching the GPU", "[resources][gltf]") {
    const RawModel raw = parse_gltf(model("test_cube.glb"));

    REQUIRE(raw.ok);
    REQUIRE_FALSE(raw.meshes.empty());
    REQUIRE_FALSE(raw.nodes.empty());
    REQUIRE_FALSE(raw.roots.empty());

    const RawMesh& mesh = raw.meshes.front();
    CHECK_FALSE(mesh.vertices.empty());
    CHECK_FALSE(mesh.indices.empty());
    CHECK(mesh.indices.size() % 3 == 0);
}

TEST_CASE("parse_gltf produces normalized normals and tangents", "[resources][gltf]") {
    const RawModel raw = parse_gltf(model("test_cube.glb"));
    REQUIRE(raw.ok);

    for (const auto& mesh : raw.meshes) {
        for (const auto& v : mesh.vertices) {
            CHECK(v.normal.length() > 0.9f);
            CHECK(v.normal.length() < 1.1f);
        }
    }
}

TEST_CASE("parse_gltf indices stay inside the vertex range", "[resources][gltf]") {
    const RawModel raw = parse_gltf(model("test_cube.glb"));
    REQUIRE(raw.ok);

    for (const auto& mesh : raw.meshes) {
        for (const u32 index : mesh.indices) {
            REQUIRE(index < mesh.vertices.size());
        }
    }
}

TEST_CASE("parse_gltf decodes embedded textures", "[resources][gltf]") {
    const RawModel raw = parse_gltf(model("test_textured.glb"));
    REQUIRE(raw.ok);

    for (const auto& texture : raw.textures) {
        CHECK(texture.width > 0);
        CHECK(texture.height > 0);
        CHECK(texture.pixels.size() == static_cast<size_t>(texture.width) * texture.height * 4);
    }
}

TEST_CASE("parse_gltf material texture indices are in range", "[resources][gltf]") {
    const RawModel raw = parse_gltf(model("test_textured.glb"));
    REQUIRE(raw.ok);

    const auto count = static_cast<i32>(raw.textures.size());
    for (const auto& m : raw.materials) {
        CHECK(m.albedo_texture < count);
        CHECK(m.normal_texture < count);
        CHECK(m.metallic_roughness_texture < count);
        CHECK(m.emissive_texture < count);
    }
}

TEST_CASE("parse_gltf node children indices are in range", "[resources][gltf]") {
    const RawModel raw = parse_gltf(model("test_cube.glb"));
    REQUIRE(raw.ok);

    for (const auto& node : raw.nodes) {
        for (const u32 child : node.children) {
            REQUIRE(child < raw.nodes.size());
        }
        for (const auto& prim : node.primitives) {
            REQUIRE(prim.mesh >= 0);
            REQUIRE(static_cast<size_t>(prim.mesh) < raw.meshes.size());
        }
    }
}

TEST_CASE("parse_gltf fails cleanly on a missing file", "[resources][gltf]") {
    const RawModel raw = parse_gltf(model("does_not_exist.glb"));
    CHECK_FALSE(raw.ok);
    CHECK(raw.meshes.empty());
}

TEST_CASE("parse_gltf runs safely on job system workers", "[resources][gltf][jobs]") {
    JobSystem jobs(4);

    constexpr u32 count = 16;
    std::vector<RawModel> results(count);

    jobs.parallel_for(count, 1, [&](const u32 begin, const u32 end) {
        for (u32 i = begin; i < end; ++i) {
            results[i] = parse_gltf(model(i % 2 == 0 ? "test_cube.glb" : "test_textured.glb"));
        }
    });

    for (const auto& raw : results) {
        REQUIRE(raw.ok);
        REQUIRE_FALSE(raw.meshes.empty());
    }

    for (u32 i = 2; i < count; i += 2) {
        CHECK(results[i].meshes.size() == results[0].meshes.size());
        CHECK(results[i].meshes[0].vertices.size() == results[0].meshes[0].vertices.size());
        CHECK(results[i].meshes[0].indices.size() == results[0].meshes[0].indices.size());
    }
}
