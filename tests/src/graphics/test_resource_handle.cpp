#include <unordered_set>

#include <catch2/catch_test_macros.hpp>

#include "star/graphics/resource_handle.hpp"

using namespace star;

using namespace star::graphics;

struct MockBuffer {};

struct MockTexture {};

struct MockShader {};

TEST_CASE("ResourceHandle - default constructed handle is invalid", "[graphics][resource_handle]") {
    constexpr ResourceHandle<MockBuffer> h;
    REQUIRE_FALSE(h.is_valid());
    REQUIRE(h.id == ResourceHandle<MockBuffer>::INVALID_ID);
    REQUIRE(h.generation == 0u);
}

TEST_CASE("ResourceHandle - handle with valid id is valid", "[graphics][resource_handle]") {
    ResourceHandle<MockBuffer> h{42u, 1u};
    REQUIRE(h.is_valid());
    REQUIRE(h.id == 42u);
    REQUIRE(h.generation == 1u);
}

TEST_CASE("ResourceHandle - INVALID_ID equals u32 max", "[graphics][resource_handle]") {
    REQUIRE(ResourceHandle<MockBuffer>::INVALID_ID == std::numeric_limits<u32>::max());
}

TEST_CASE("ResourceHandle - reset makes handle invalid", "[graphics][resource_handle]") {
    ResourceHandle<MockBuffer> h{1u, 3u};
    h.reset();
    REQUIRE_FALSE(h.is_valid());
    REQUIRE(h.id == ResourceHandle<MockBuffer>::INVALID_ID);
    REQUIRE(h.generation == 0u);
}

TEST_CASE("ResourceHandle - same id and generation are equal", "[graphics][resource_handle]") {
    ResourceHandle<MockBuffer> a{10u, 1u};
    ResourceHandle<MockBuffer> b{10u, 1u};
    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
}

TEST_CASE("ResourceHandle - different generation means not equal", "[graphics][resource_handle]") {
    ResourceHandle<MockBuffer> a{10u, 1u};
    ResourceHandle<MockBuffer> b{10u, 2u};
    REQUIRE(a != b);
}

TEST_CASE("ResourceHandle - different id means not equal", "[graphics][resource_handle]") {
    ResourceHandle<MockBuffer> a{1u, 0u};
    ResourceHandle<MockBuffer> b{2u, 0u};
    REQUIRE(a != b);
}

TEST_CASE("ResourceHandle - two default handles are equal", "[graphics][resource_handle]") {
    ResourceHandle<MockBuffer> a;
    ResourceHandle<MockBuffer> b;
    REQUIRE(a == b);
}

TEST_CASE("ResourceHandle - same id in different types does not compare", "[graphics][resource_handle]") {
    constexpr ResourceHandle<MockBuffer> buf{5u, 0u};
    constexpr ResourceHandle<MockTexture> tex{5u, 0u};
    REQUIRE(buf.id == tex.id);
    REQUIRE(buf.is_valid());
    REQUIRE(tex.is_valid());
}

TEST_CASE("ResourceHandle - can be stored in unordered_set", "[graphics][resource_handle]") {
    std::unordered_set<ResourceHandle<MockBuffer>> set;
    ResourceHandle<MockBuffer> a{1u, 0u};
    ResourceHandle<MockBuffer> b{2u, 0u};
    set.insert(a);
    set.insert(b);
    REQUIRE(set.size() == 2u);
    REQUIRE(set.contains(a));
    REQUIRE(set.contains(b));
}

TEST_CASE("ResourceHandle - distinct handles produce distinct hashes", "[graphics][resource_handle]") {
    constexpr std::hash<ResourceHandle<MockBuffer>> hasher;
    constexpr ResourceHandle<MockBuffer> a{1u, 0u};
    constexpr ResourceHandle<MockBuffer> b{2u, 0u};
    REQUIRE(hasher(a) != hasher(b));
}

TEST_CASE("ResourceHandle - same handle always produces same hash", "[graphics][resource_handle]") {
    constexpr std::hash<ResourceHandle<MockBuffer>> hasher;
    constexpr ResourceHandle<MockBuffer> h{7u, 3u};
    REQUIRE(hasher(h) == hasher(h));
}
