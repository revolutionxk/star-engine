#include <set>
#include <unordered_set>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "star/core/job_system.hpp"
#include "star/core/uuid.hpp"

using namespace star;

TEST_CASE("default UUID is invalid", "[core][uuid]") {
    constexpr UUID id;
    STATIC_REQUIRE_FALSE(id.is_valid());
}

TEST_CASE("generated Uuids are valid and distinct", "[core][uuid]") {
    std::unordered_set<UUID> seen;
    for (int i = 0; i < 10000; ++i) {
        const UUID id = UUID::generate();
        REQUIRE(id.is_valid());
        REQUIRE(seen.insert(id).second);
    }
}

TEST_CASE("UUID round-trips through text", "[core][uuid]") {
    for (int i = 0; i < 1000; ++i) {
        const UUID id = UUID::generate();
        const std::string text = id.to_string();

        REQUIRE(text.size() == 32);

        const auto parsed = UUID::parse(text);
        REQUIRE(parsed.has_value());
        CHECK(*parsed == id);
    }
}

TEST_CASE("UUID::parse rejects malformed text", "[core][uuid]") {
    CHECK_FALSE(UUID::parse("").has_value());
    CHECK_FALSE(UUID::parse("abc").has_value());
    CHECK_FALSE(UUID::parse(std::string(31, 'a')).has_value());
    CHECK_FALSE(UUID::parse(std::string(33, 'a')).has_value());
    CHECK_FALSE(UUID::parse(std::string(32, 'z')).has_value());
    CHECK_FALSE(UUID::parse("0123456789abcdef0123456789abcdeg").has_value());
}

TEST_CASE("UUID::parse accepts an all-zero string as the invalid id", "[core][uuid]") {
    const auto parsed = UUID::parse(std::string(32, '0'));
    REQUIRE(parsed.has_value());
    CHECK_FALSE(parsed->is_valid());
}

TEST_CASE("derive is deterministic and tag-sensitive", "[core][uuid]") {
    const UUID parent = UUID::generate();

    const UUID a = UUID::derive(parent, "m0_p0");
    const UUID b = UUID::derive(parent, "m0_p0");
    const UUID c = UUID::derive(parent, "m0_p1");

    CHECK(a == b);
    CHECK(a != c);
    CHECK(a.is_valid());
    CHECK(a != parent);
}

TEST_CASE("derive from different parents does not collide", "[core][uuid]") {
    const UUID first = UUID::generate();
    const UUID second = UUID::generate();

    CHECK(UUID::derive(first, "mesh") != UUID::derive(second, "mesh"));
}

TEST_CASE("derive spreads across many tags without collision", "[core][uuid]") {
    const UUID parent = UUID::generate();

    std::unordered_set<UUID> seen;
    for (int i = 0; i < 5000; ++i) {
        REQUIRE(seen.insert(UUID::derive(parent, "sub" + std::to_string(i))).second);
    }
}

TEST_CASE("UUID is ordered and usable as a map key", "[core][uuid]") {
    std::set<UUID> ordered;
    for (int i = 0; i < 100; ++i) {
        ordered.insert(UUID::generate());
    }
    CHECK(ordered.size() == 100);
}

TEST_CASE("generate is safe from multiple threads", "[core][uuid][jobs]") {
    JobSystem jobs(4);

    constexpr u32 per_chunk = 2000;
    constexpr u32 chunks = 8;
    std::vector<std::vector<UUID>> buckets(chunks);

    jobs.parallel_for(chunks, 1, [&](const u32 begin, const u32 end) {
        for (u32 c = begin; c < end; ++c) {
            buckets[c].reserve(per_chunk);
            for (u32 i = 0; i < per_chunk; ++i) {
                buckets[c].push_back(UUID::generate());
            }
        }
    });

    std::unordered_set<UUID> all;
    for (const auto& bucket : buckets) {
        for (const UUID& id : bucket) {
            REQUIRE(id.is_valid());
            REQUIRE(all.insert(id).second);
        }
    }
    CHECK(all.size() == static_cast<std::size_t>(per_chunk) * chunks);
}
