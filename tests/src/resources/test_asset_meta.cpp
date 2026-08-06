#include <filesystem>
#include <fstream>

#include <catch2/catch_test_macros.hpp>

#include "star/resources/asset_meta.hpp"

using namespace star;
using namespace star::resources;

namespace star::resources::test {
    std::filesystem::path scratch_dir() {
        auto dir = std::filesystem::temp_directory_path() / "star_asset_meta_tests";
        std::filesystem::create_directories(dir);
        return dir;
    }

    std::filesystem::path make_asset(const std::string& name) {
        const auto path = scratch_dir() / name;
        std::ofstream out(path);
        out << "payload";
        return path;
    }

    void clean(const std::filesystem::path& asset) {
        std::error_code ec;
        std::filesystem::remove(meta_path_for(asset), ec);
        std::filesystem::remove(asset, ec);
    }
} // namespace star::resources::test

TEST_CASE("meta_path_for appends .meta to the full filename", "[resources][meta]") {
    CHECK(meta_path_for("/a/b/model.glb").filename() == "model.glb.meta");
    CHECK(meta_path_for("/a/b/tex.png").filename() == "tex.png.meta");
}

TEST_CASE("read_asset_uuid returns invalid when there is no meta", "[resources][meta]") {
    const auto asset = test::make_asset("no_meta.png");
    CHECK_FALSE(read_asset_uuid(asset).is_valid());
    test::clean(asset);
}

TEST_CASE("ensure_asset_uuid creates a meta and is stable across calls", "[resources][meta]") {
    const auto asset = test::make_asset("stable.png");

    const Uuid first = ensure_asset_uuid(asset);
    REQUIRE(first.is_valid());
    REQUIRE(std::filesystem::exists(meta_path_for(asset)));

    const Uuid second = ensure_asset_uuid(asset);
    CHECK(second == first);

    test::clean(asset);
}

TEST_CASE("uuid survives renaming the asset when the meta follows", "[resources][meta]") {
    const auto original = test::make_asset("before.png");
    const Uuid id = ensure_asset_uuid(original);

    const auto renamed = test::scratch_dir() / "after.png";
    std::filesystem::rename(original, renamed);
    std::filesystem::rename(meta_path_for(original), meta_path_for(renamed));

    CHECK(read_asset_uuid(renamed) == id);

    test::clean(renamed);
}

TEST_CASE("a malformed meta is replaced instead of crashing", "[resources][meta]") {
    const auto asset = test::make_asset("broken.png");
    {
        std::ofstream out(meta_path_for(asset));
        out << "{ not json";
    }

    CHECK_FALSE(read_asset_uuid(asset).is_valid());

    const Uuid recovered = ensure_asset_uuid(asset);
    CHECK(recovered.is_valid());
    CHECK(read_asset_uuid(asset) == recovered);

    test::clean(asset);
}

TEST_CASE("a meta without a uuid field is treated as missing", "[resources][meta]") {
    const auto asset = test::make_asset("nouuid.png");
    {
        std::ofstream out(meta_path_for(asset));
        out << R"({"version": 1})";
    }

    CHECK_FALSE(read_asset_uuid(asset).is_valid());
    test::clean(asset);
}

TEST_CASE("distinct assets get distinct uuids", "[resources][meta]") {
    const auto a = test::make_asset("one.png");
    const auto b = test::make_asset("two.png");

    CHECK(ensure_asset_uuid(a) != ensure_asset_uuid(b));

    test::clean(a);
    test::clean(b);
}
