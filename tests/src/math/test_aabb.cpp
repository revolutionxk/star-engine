#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "star/math/math.hpp"

using namespace star;

TEST_CASE("AABB - default construction produces an invalid (empty) box", "[math][aabb]") {
    constexpr AABB box;
    REQUIRE_FALSE(box.is_valid());
}

TEST_CASE("AABB - min/max construction", "[math][aabb]") {
    constexpr AABB box{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};
    REQUIRE(box.is_valid());
    REQUIRE(box.center() == Vector3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("AABB - from_center_extents", "[math][aabb]") {
    AABB box = AABB::from_center_extents({0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});
    REQUIRE(box.min == Vector3{-1.0f, -1.0f, -1.0f});
    REQUIRE(box.max == Vector3{1.0f, 1.0f, 1.0f});
}

TEST_CASE("AABB - from_point creates a degenerate box at that point", "[math][aabb]") {
    AABB box = AABB::from_point({2.0f, 3.0f, 4.0f});
    REQUIRE(box.min == box.max);
    REQUIRE(box.is_empty());
}

TEST_CASE("AABB - center is midpoint of min and max", "[math][aabb]") {
    constexpr AABB box{{0.0f, 0.0f, 0.0f}, {4.0f, 6.0f, 8.0f}};
    constexpr Vector3 c = box.center();
    REQUIRE(c.x == Catch::Approx(2.0f));
    REQUIRE(c.y == Catch::Approx(3.0f));
    REQUIRE(c.z == Catch::Approx(4.0f));
}

TEST_CASE("AABB - extents returns size per axis", "[math][aabb]") {
    constexpr AABB box{{0.0f, 0.0f, 0.0f}, {4.0f, 6.0f, 8.0f}};
    constexpr Vector3 e = box.extents();
    REQUIRE(e.x == Catch::Approx(4.0f));
    REQUIRE(e.y == Catch::Approx(6.0f));
    REQUIRE(e.z == Catch::Approx(8.0f));
}

TEST_CASE("AABB - half_extents is half of extents", "[math][aabb]") {
    constexpr AABB box{{0.0f, 0.0f, 0.0f}, {4.0f, 6.0f, 8.0f}};
    constexpr Vector3 he = box.half_extents();
    REQUIRE(he.x == Catch::Approx(2.0f));
    REQUIRE(he.y == Catch::Approx(3.0f));
    REQUIRE(he.z == Catch::Approx(4.0f));
}

TEST_CASE("AABB - volume of unit cube is 1", "[math][aabb]") {
    constexpr AABB box{{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};
    REQUIRE(box.volume() == Catch::Approx(1.0f));
}

TEST_CASE("AABB - volume of 2x3x4 box is 24", "[math][aabb]") {
    constexpr AABB box{{0.0f, 0.0f, 0.0f}, {2.0f, 3.0f, 4.0f}};
    REQUIRE(box.volume() == Catch::Approx(24.0f));
}

TEST_CASE("AABB - surface area of unit cube is 6", "[math][aabb]") {
    constexpr AABB box{{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};
    REQUIRE(box.surface_area() == Catch::Approx(6.0f));
}

TEST_CASE("AABB - contains point inside", "[math][aabb]") {
    constexpr AABB box{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};
    REQUIRE(box.contains({0.0f, 0.0f, 0.0f}));
}

TEST_CASE("AABB - contains point on boundary", "[math][aabb]") {
    constexpr AABB box{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};
    REQUIRE(box.contains({1.0f, 1.0f, 1.0f}));
    REQUIRE(box.contains({-1.0f, -1.0f, -1.0f}));
}

TEST_CASE("AABB - does not contain point outside", "[math][aabb]") {
    constexpr AABB box{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};
    REQUIRE_FALSE(box.contains({2.0f, 0.0f, 0.0f}));
    REQUIRE_FALSE(box.contains({0.0f, -2.0f, 0.0f}));
}

TEST_CASE("AABB - contains smaller box", "[math][aabb]") {
    constexpr AABB outer{{-2.0f, -2.0f, -2.0f}, {2.0f, 2.0f, 2.0f}};
    constexpr AABB inner{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};
    REQUIRE(outer.contains(inner));
    REQUIRE_FALSE(inner.contains(outer));
}

TEST_CASE("AABB - overlapping boxes intersect", "[math][aabb]") {
    constexpr AABB a{{0.0f, 0.0f, 0.0f}, {2.0f, 2.0f, 2.0f}};
    constexpr AABB b{{1.0f, 1.0f, 1.0f}, {3.0f, 3.0f, 3.0f}};
    REQUIRE(a.intersects(b));
    REQUIRE(b.intersects(a));
}

TEST_CASE("AABB - non-overlapping boxes do not intersect", "[math][aabb]") {
    constexpr AABB a{{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};
    constexpr AABB b{{5.0f, 5.0f, 5.0f}, {6.0f, 6.0f, 6.0f}};
    REQUIRE_FALSE(a.intersects(b));
}

TEST_CASE("AABB - touching boxes intersect at boundary", "[math][aabb]") {
    constexpr AABB a{{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};
    constexpr AABB b{{1.0f, 0.0f, 0.0f}, {2.0f, 1.0f, 1.0f}};
    REQUIRE(a.intersects(b));
}

TEST_CASE("AABB - expand by point grows the box", "[math][aabb]") {
    AABB box = AABB::from_point({0.0f, 0.0f, 0.0f});
    box.expand({3.0f, 4.0f, 5.0f});
    REQUIRE(box.max.x == Catch::Approx(3.0f));
    REQUIRE(box.max.y == Catch::Approx(4.0f));
    REQUIRE(box.max.z == Catch::Approx(5.0f));
    REQUIRE(box.contains({1.0f, 2.0f, 3.0f}));
}
