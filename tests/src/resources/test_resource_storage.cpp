#include <memory>

#include <catch2/catch_test_macros.hpp>

#include "star/resources/resource_manager.hpp"

using namespace star;
using namespace star::resources;

TEST_CASE("ResourceStorage recycles ids with a bumped generation", "[resources][handles]") {
    ResourceStorage<int> storage;

    const u32 id = storage.allocate_id();
    const u32 gen = storage.generation_of(id);
    storage.resources[id] = {std::make_unique<int>(42), "answer", gen, 1};
    REQUIRE(gen == 1);
    REQUIRE(storage.resources[id].name == "answer");

    storage.resources.erase(id);
    storage.release_id(id);

    const u32 recycled = storage.allocate_id();
    const u32 new_gen = storage.generation_of(recycled);

    CHECK(recycled == id);
    CHECK(new_gen == gen + 1);
    CHECK(new_gen != gen);
}

TEST_CASE("ResourceStorage generation is stable until release", "[resources][handles]") {
    ResourceStorage<int> storage;

    const u32 id = storage.allocate_id();
    CHECK(storage.generation_of(id) == 1);
    CHECK(storage.generation_of(id) == 1);

    storage.release_id(id);
    CHECK(storage.generation_of(id) == 2);
}
