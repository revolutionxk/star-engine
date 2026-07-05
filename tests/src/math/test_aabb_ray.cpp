#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "star/math/aabb.hpp"

using namespace star;

TEST_CASE("AABB::intersect_ray hits, misses and starts inside", "[math][aabb]") {
    constexpr AABB box{Vector3{-1.0f, -1.0f, -1.0f}, Vector3{1.0f, 1.0f, 1.0f}};

    SECTION("head-on hit returns the entry distance") {
        const auto t = box.intersect_ray(Vector3{0.0f, 0.0f, -5.0f}, Vector3{0.0f, 0.0f, 1.0f});
        REQUIRE(t.has_value());
        CHECK(*t == Catch::Approx(4.0f)); // enters the front face at z = -1
    }

    SECTION("ray pointing away misses") {
        CHECK_FALSE(box.intersect_ray(Vector3{0.0f, 0.0f, -5.0f}, Vector3{0.0f, 0.0f, -1.0f}).has_value());
    }

    SECTION("ray beside the box misses") {
        CHECK_FALSE(box.intersect_ray(Vector3{5.0f, 5.0f, -5.0f}, Vector3{0.0f, 0.0f, 1.0f}).has_value());
    }

    SECTION("ray parallel to a slab and outside misses") {
        CHECK_FALSE(box.intersect_ray(Vector3{2.0f, 0.0f, -5.0f}, Vector3{0.0f, 0.0f, 1.0f}).has_value());
    }

    SECTION("origin inside the box hits at t = 0") {
        const auto t = box.intersect_ray(Vector3{0.0f, 0.0f, 0.0f}, Vector3{1.0f, 0.0f, 0.0f});
        REQUIRE(t.has_value());
        CHECK(*t == Catch::Approx(0.0f));
    }
}
