#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "star/math/common.hpp"

using namespace star::math;

TEST_CASE("Math constants - pi", "[math][common]") {
    REQUIRE(Constantsf::pi == Catch::Approx(3.14159f).margin(1e-4f));
    REQUIRE(Constantsf::two_pi == Catch::Approx(6.28318f).margin(1e-4f));
    REQUIRE(Constantsf::half_pi == Catch::Approx(1.57079f).margin(1e-4f));
    REQUIRE(Constantsf::pi * 2.0f == Catch::Approx(Constantsf::two_pi).margin(1e-5f));
}

TEST_CASE("Math constants - sqrt2 and sqrt3", "[math][common]") {
    REQUIRE(Constantsf::sqrt2 * Constantsf::sqrt2 == Catch::Approx(2.0f).margin(1e-5f));
    REQUIRE(Constantsf::sqrt3 * Constantsf::sqrt3 == Catch::Approx(3.0f).margin(1e-4f));
}

TEST_CASE("Math constants - deg_to_rad and rad_to_deg are reciprocals", "[math][common]") {
    REQUIRE(Constantsf::deg_to_rad * Constantsf::rad_to_deg == Catch::Approx(1.0f).margin(1e-5f));
}

TEST_CASE("radians converts degrees correctly", "[math][common]") {
    REQUIRE(radians(0.0f) == Catch::Approx(0.0f));
    REQUIRE(radians(180.0f) == Catch::Approx(Constantsf::pi).margin(1e-5f));
    REQUIRE(radians(360.0f) == Catch::Approx(Constantsf::two_pi).margin(1e-5f));
    REQUIRE(radians(90.0f) == Catch::Approx(Constantsf::half_pi).margin(1e-5f));
}

TEST_CASE("degrees converts radians correctly", "[math][common]") {
    REQUIRE(degrees(0.0f) == Catch::Approx(0.0f));
    REQUIRE(degrees(Constantsf::pi) == Catch::Approx(180.0f).margin(1e-4f));
    REQUIRE(degrees(Constantsf::two_pi) == Catch::Approx(360.0f).margin(1e-4f));
}

TEST_CASE("radians and degrees are inverse operations", "[math][common]") {
    REQUIRE(degrees(radians(45.0f)) == Catch::Approx(45.0f).margin(1e-4f));
    REQUIRE(radians(degrees(Constantsf::pi)) == Catch::Approx(Constantsf::pi).margin(1e-5f));
}

TEST_CASE("lerp at t=0 returns a", "[math][common]") {
    REQUIRE(lerp(0.0f, 10.0f, 0.0f) == Catch::Approx(0.0f));
}

TEST_CASE("lerp at t=1 returns b", "[math][common]") {
    REQUIRE(lerp(0.0f, 10.0f, 1.0f) == Catch::Approx(10.0f));
}

TEST_CASE("lerp at t=0.5 returns midpoint", "[math][common]") {
    REQUIRE(lerp(0.0f, 10.0f, 0.5f) == Catch::Approx(5.0f));
    REQUIRE(lerp(2.0f, 4.0f, 0.25f) == Catch::Approx(2.5f));
}

TEST_CASE("clamp - value inside range is unchanged", "[math][common]") {
    REQUIRE(clamp(5.0f, 0.0f, 10.0f) == Catch::Approx(5.0f));
}

TEST_CASE("clamp - value below min is clamped to min", "[math][common]") {
    REQUIRE(clamp(-5.0f, 0.0f, 10.0f) == Catch::Approx(0.0f));
}

TEST_CASE("clamp - value above max is clamped to max", "[math][common]") {
    REQUIRE(clamp(15.0f, 0.0f, 10.0f) == Catch::Approx(10.0f));
}

TEST_CASE("clamp - boundary values are preserved", "[math][common]") {
    REQUIRE(clamp(0.0f, 0.0f, 10.0f) == Catch::Approx(0.0f));
    REQUIRE(clamp(10.0f, 0.0f, 10.0f) == Catch::Approx(10.0f));
}

TEST_CASE("approximately - equal values return true", "[math][common]") {
    REQUIRE(approximately(1.0f, 1.0f));
}

TEST_CASE("approximately - values within epsilon return true", "[math][common]") {
    REQUIRE(approximately(1.0f, 1.0f + Constantsf::epsilon * 0.5f));
}

TEST_CASE("approximately - values outside epsilon return false", "[math][common]") {
    REQUIRE_FALSE(approximately(1.0f, 2.0f));
    REQUIRE_FALSE(approximately(0.0f, 0.001f));
}

TEST_CASE("approximately - custom epsilon", "[math][common]") {
    REQUIRE(approximately(1.0f, 1.1f, 0.2f));
    REQUIRE_FALSE(approximately(1.0f, 1.3f, 0.2f));
}

TEST_CASE("sign - positive value returns 1", "[math][common]") {
    REQUIRE(sign(5.0f) == 1.0f);
}

TEST_CASE("sign - negative value returns -1", "[math][common]") {
    REQUIRE(sign(-3.0f) == -1.0f);
}

TEST_CASE("sign - zero returns 0", "[math][common]") {
    REQUIRE(sign(0.0f) == 0.0f);
}

TEST_CASE("smoothstep - at edge0 returns 0", "[math][common]") {
    REQUIRE(smoothstep(0.0f, 1.0f, 0.0f) == Catch::Approx(0.0f));
}

TEST_CASE("smoothstep - at edge1 returns 1", "[math][common]") {
    REQUIRE(smoothstep(0.0f, 1.0f, 1.0f) == Catch::Approx(1.0f));
}

TEST_CASE("smoothstep - at midpoint returns 0.5", "[math][common]") {
    REQUIRE(smoothstep(0.0f, 1.0f, 0.5f) == Catch::Approx(0.5f));
}

TEST_CASE("smoothstep - clamps below edge0", "[math][common]") {
    REQUIRE(smoothstep(0.0f, 1.0f, -1.0f) == Catch::Approx(0.0f));
}

TEST_CASE("smoothstep - clamps above edge1", "[math][common]") {
    REQUIRE(smoothstep(0.0f, 1.0f, 2.0f) == Catch::Approx(1.0f));
}

TEST_CASE("smootherstep - at edges returns 0 and 1", "[math][common]") {
    REQUIRE(smootherstep(0.0f, 1.0f, 0.0f) == Catch::Approx(0.0f));
    REQUIRE(smootherstep(0.0f, 1.0f, 1.0f) == Catch::Approx(1.0f));
}

TEST_CASE("smootherstep - at midpoint returns 0.5", "[math][common]") {
    REQUIRE(smootherstep(0.0f, 1.0f, 0.5f) == Catch::Approx(0.5f));
}
