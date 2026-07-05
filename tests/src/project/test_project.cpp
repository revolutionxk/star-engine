#include <filesystem>

#include <catch2/catch_test_macros.hpp>

#include "star/project/project.hpp"

using namespace star;

namespace {
    std::filesystem::path scratch_dir() {
        return std::filesystem::temp_directory_path() / "star_engine_project_tests";
    }
} // namespace

TEST_CASE("Project::create scaffolds the standard layout and manifest", "[project]") {
    const auto parent = scratch_dir();
    std::filesystem::remove_all(parent);
    std::filesystem::create_directories(parent);

    const auto project = project::Project::create(parent, "MyGame");
    REQUIRE(project.has_value());

    const auto root = parent / "MyGame";
    CHECK(std::filesystem::is_directory(root / "Assets"));
    CHECK(std::filesystem::is_directory(root / "Assets" / "Scenes"));
    CHECK(std::filesystem::is_directory(root / "Config"));
    CHECK(std::filesystem::is_directory(root / "Cache"));
    CHECK(std::filesystem::exists(root / "MyGame.starproject"));
    CHECK(project->name() == "MyGame");
    CHECK(project->default_scene() == "Assets/Scenes/Main.starscene");

    std::filesystem::remove_all(parent);
}

TEST_CASE("Project::create refuses to overwrite an existing directory", "[project]") {
    const auto parent = scratch_dir();
    std::filesystem::remove_all(parent);
    std::filesystem::create_directories(parent / "Taken");

    CHECK_FALSE(project::Project::create(parent, "Taken").has_value());

    std::filesystem::remove_all(parent);
}

TEST_CASE("Project round-trips through save/load", "[project]") {
    const auto parent = scratch_dir();
    std::filesystem::remove_all(parent);
    std::filesystem::create_directories(parent);

    auto created = project::Project::create(parent, "RoundTrip");
    REQUIRE(created.has_value());
    created->set_default_scene("Assets/Scenes/Level2.starscene");
    REQUIRE(created->save());

    const auto loaded = project::Project::load(parent / "RoundTrip");
    REQUIRE(loaded.has_value());
    CHECK(loaded->name() == "RoundTrip");
    CHECK(loaded->default_scene() == "Assets/Scenes/Level2.starscene");
    CHECK(loaded->root() == created->root());

    std::filesystem::remove_all(parent);
}

TEST_CASE("Project path helpers resolve relative and absolute paths", "[project]") {
    const auto parent = scratch_dir();
    std::filesystem::remove_all(parent);
    std::filesystem::create_directories(parent);

    const auto project = project::Project::create(parent, "Paths");
    REQUIRE(project.has_value());

    const auto scene = project->resolve("Assets/Scenes/Main.starscene");
    CHECK(scene == project->scenes_dir() / "Main.starscene");
    CHECK(project->to_relative(scene) == std::filesystem::path("Assets/Scenes/Main.starscene"));
    
    const auto outside = std::filesystem::temp_directory_path() / "elsewhere.txt";
    CHECK(project->to_relative(outside) == outside);

    std::filesystem::remove_all(parent);
}
