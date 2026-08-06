#include <set>
#include <unordered_set>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "star/core/job_system.hpp"
#include "star/core/uuid.hpp"

using namespace star;

TEST_CASE("default Uuid is invalid", "[core][uuid]") {
    constexpr Uuid id;
    STATIC_REQUIRE_FALSE(id.is_valid());
}

TEST_CASE("generated Uuids are valid and distinct", "[core][uuid]") {
    std::unordered_set<Uuid> seen;
    for (int i = 0; i < 10000; ++i) {
        const Uuid id = Uuid::generate();
        REQUIRE(id.is_valid());
        REQUIRE(seen.insert(id).second);
    }
}

TEST_CASE("Uuid round-trips through text", "[core][uuid]") {
    for (int i = 0; i < 1000; ++i) {
        const Uuid id = Uuid::generate();
        const std::string text = id.to_string();

        REQUIRE(text.size() == 32);

        const auto parsed = Uuid::parse(text);
        REQUIRE(parsed.has_value());
        CHECK(*parsed == id);
    }
}

TEST_CASE("Uuid::parse rejects malformed text", "[core][uuid]") {
    CHECK_FALSE(Uuid::parse("").has_value());
    CHECK_FALSE(Uuid::parse("abc").has_value());
    CHECK_FALSE(Uuid::parse(std::string(31, 'a')).has_value());
    CHECK_FALSE(Uuid::parse(std::string(33, 'a')).has_value());
    CHECK_FALSE(Uuid::parse(std::string(32, 'z')).has_value());
    CHECK_FALSE(Uuid::parse("0123456789abcdef0123456789abcdeg").has_value());
}

TEST_CASE("Uuid::parse accepts an all-zero string as the invalid id", "[core][uuid]") {
    const auto parsed = Uuid::parse(std::string(32, '0'));
    REQUIRE(parsed.has_value());
    CHECK_FALSE(parsed->is_valid());
}

TEST_CASE("derive is deterministic and tag-sensitive", "[core][uuid]") {
    const Uuid parent = Uuid::generate();

    const Uuid a = Uuid::derive(parent, "m0_p0");
    const Uuid b = Uuid::derive(parent, "m0_p0");
    const Uuid c = Uuid::derive(parent, "m0_p1");

    CHECK(a == b);
    CHECK(a != c);
    CHECK(a.is_valid());
    CHECK(a != parent);
}

TEST_CASE("derive from different parents does not collide", "[core][uuid]") {
    const Uuid first = Uuid::generate();
    const Uuid second = Uuid::generate();

    CHECK(Uuid::derive(first, "mesh") != Uuid::derive(second, "mesh"));
}

TEST_CASE("derive spreads across many tags without collision", "[core][uuid]") {
    const Uuid parent = Uuid::generate();

    std::unordered_set<Uuid> seen;
    for (int i = 0; i < 5000; ++i) {
        REQUIRE(seen.insert(Uuid::derive(parent, "sub" + std::to_string(i))).second);
    }
}

TEST_CASE("Uuid is ordered and usable as a map key", "[core][uuid]") {
    std::set<Uuid> ordered;
    for (int i = 0; i < 100; ++i) {
        ordered.insert(Uuid::generate());
    }
    CHECK(ordered.size() == 100);
}

TEST_CASE("generate is safe from multiple threads", "[core][uuid][jobs]") {
    JobSystem jobs(4);

    constexpr u32 per_chunk = 2000;
    constexpr u32 chunks = 8;
    std::vector<std::vector<Uuid>> buckets(chunks);

    jobs.parallel_for(chunks, 1, [&](const u32 begin, const u32 end) {
        for (u32 c = begin; c < end; ++c) {
            buckets[c].reserve(per_chunk);
            for (u32 i = 0; i < per_chunk; ++i) {
                buckets[c].push_back(Uuid::generate());
            }
        }
    });

    std::unordered_set<Uuid> all;
    for (const auto& bucket : buckets) {
        for (const Uuid& id : bucket) {
            REQUIRE(id.is_valid());
            REQUIRE(all.insert(id).second);
        }
    }
    CHECK(all.size() == static_cast<std::size_t>(per_chunk) * chunks);
}
