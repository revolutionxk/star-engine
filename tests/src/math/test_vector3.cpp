#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "star/math/math.hpp"

using namespace star;

TEST_CASE("Vector3 - default construction is zero", "[math][vector3]") {
    constexpr Vector3 v;
    REQUIRE(v.x == 0.0f);
    REQUIRE(v.y == 0.0f);
    REQUIRE(v.z == 0.0f);
}

TEST_CASE("Vector3 - component construction", "[math][vector3]") {
    constexpr Vector3 v{1.0f, 2.0f, 3.0f};
    REQUIRE(v.x == 1.0f);
    REQUIRE(v.y == 2.0f);
    REQUIRE(v.z == 3.0f);
}

TEST_CASE("Vector3 - scalar broadcast construction", "[math][vector3]") {
    constexpr Vector3 v{5.0f};
    REQUIRE(v.x == 5.0f);
    REQUIRE(v.y == 5.0f);
    REQUIRE(v.z == 5.0f);
}

TEST_CASE("Vector3 - static factories", "[math][vector3]") {
    REQUIRE(Vector3::zero() == Vector3{0.0f, 0.0f, 0.0f});
    REQUIRE(Vector3::one() == Vector3{1.0f, 1.0f, 1.0f});
    REQUIRE(Vector3::right() == Vector3{1.0f, 0.0f, 0.0f});
    REQUIRE(Vector3::left() == Vector3{-1.0f, 0.0f, 0.0f});
    REQUIRE(Vector3::up() == Vector3{0.0f, 1.0f, 0.0f});
    REQUIRE(Vector3::down() == Vector3{0.0f, -1.0f, 0.0f});
    REQUIRE(Vector3::forward() == Vector3{0.0f, 0.0f, -1.0f});
    REQUIRE(Vector3::back() == Vector3{0.0f, 0.0f, 1.0f});
}

TEST_CASE("Vector3 - arithmetic operators", "[math][vector3]") {
    Vector3 a{1.0f, 2.0f, 3.0f};
    Vector3 b{4.0f, 5.0f, 6.0f};

    SECTION("addition") {
        REQUIRE(a + b == Vector3{5.0f, 7.0f, 9.0f});
    }
    SECTION("subtraction") {
        REQUIRE(b - a == Vector3{3.0f, 3.0f, 3.0f});
    }
    SECTION("scalar multiply") {
        REQUIRE(a * 2.0f == Vector3{2.0f, 4.0f, 6.0f});
    }
    SECTION("component multiply") {
        REQUIRE(a * b == Vector3{4.0f, 10.0f, 18.0f});
    }
    SECTION("scalar divide") {
        REQUIRE(b / 2.0f == Vector3{2.0f, 2.5f, 3.0f});
    }
    SECTION("negate") {
        REQUIRE(-a == Vector3{-1.0f, -2.0f, -3.0f});
    }
}

TEST_CASE("Vector3 - compound assignment operators", "[math][vector3]") {
    Vector3 v{1.0f, 2.0f, 3.0f};

    v += {1.0f, 1.0f, 1.0f};
    REQUIRE(v == Vector3{2.0f, 3.0f, 4.0f});

    v -= {1.0f, 1.0f, 1.0f};
    REQUIRE(v == Vector3{1.0f, 2.0f, 3.0f});

    v *= 2.0f;
    REQUIRE(v == Vector3{2.0f, 4.0f, 6.0f});

    v /= 2.0f;
    REQUIRE(v == Vector3{1.0f, 2.0f, 3.0f});
}

TEST_CASE("Vector3 - dot product", "[math][vector3]") {
    REQUIRE(Vector3::right().dot(Vector3::up()) == Catch::Approx(0.0f));
    REQUIRE(Vector3::right().dot(Vector3::right()) == Catch::Approx(1.0f));
    REQUIRE(Vector3{1, 2, 3}.dot({4, 5, 6}) == Catch::Approx(32.0f));
    REQUIRE(Vector3{1, 0, 0}.dot({-1, 0, 0}) == Catch::Approx(-1.0f));
}

TEST_CASE("Vector3 - cross product", "[math][vector3]") {
    REQUIRE(Vector3::right().cross(Vector3::up()) == Vector3::back());
    REQUIRE(Vector3::up().cross(Vector3::right()) == Vector3::forward());
    REQUIRE(Vector3::up().cross(Vector3::forward()) == Vector3::left());
}

TEST_CASE("Vector3 - cross product of parallel vectors is zero", "[math][vector3]") {
    Vector3 cross = Vector3::right().cross(Vector3::right());
    REQUIRE(cross == Vector3::zero());
}

TEST_CASE("Vector3 - length squared", "[math][vector3]") {
    REQUIRE(Vector3{1.0f, 2.0f, 2.0f}.length_squared() == Catch::Approx(9.0f));
}

TEST_CASE("Vector3 - length", "[math][vector3]") {
    REQUIRE(Vector3{1.0f, 2.0f, 2.0f}.length() == Catch::Approx(3.0f));
}

TEST_CASE("Vector3 - normalized returns unit vector", "[math][vector3]") {
    Vector3 n = Vector3{2.0f, 0.0f, 0.0f}.normalized();
    REQUIRE(n.length() == Catch::Approx(1.0f).margin(1e-5f));
    REQUIRE(n.is_normalized());
}

TEST_CASE("Vector3 - normalizing zero vector returns zero", "[math][vector3]") {
    REQUIRE(Vector3::zero().normalized() == Vector3::zero());
}

TEST_CASE("Vector3 - normalize in-place", "[math][vector3]") {
    Vector3 v{0.0f, 3.0f, 4.0f};
    v.normalize();
    REQUIRE(v.is_normalized());
}

TEST_CASE("Vector3 - distance_to", "[math][vector3]") {
    REQUIRE(Vector3{0, 0, 0}.distance_to({1, 0, 0}) == Catch::Approx(1.0f));
    REQUIRE(Vector3{0, 0, 0}.distance_to({3, 4, 0}) == Catch::Approx(5.0f));
}

TEST_CASE("Vector3 - distance_squared_to", "[math][vector3]") {
    REQUIRE(Vector3{0, 0, 0}.distance_squared_to({1, 2, 2}) == Catch::Approx(9.0f));
}

TEST_CASE("Vector3 - lerp", "[math][vector3]") {
    Vector3 a{0, 0, 0};
    Vector3 b{1, 1, 1};
    REQUIRE(a.lerp(b, 0.0f) == a);
    REQUIRE(a.lerp(b, 1.0f) == b);
    REQUIRE(a.lerp(b, 0.5f) == Vector3{0.5f, 0.5f, 0.5f});
}

TEST_CASE("Vector3 - reflect mirrors across normal", "[math][vector3]") {
    // Incident ray going downward, reflecting off the ground (Y+ normal)
    constexpr auto incident = Vector3{0.0f, -1.0f, 0.0f};
    constexpr Vector3 reflected = incident.reflect(Vector3::up());
    REQUIRE(reflected == Vector3{0.0f, 1.0f, 0.0f});
}

TEST_CASE("Vector3 - project onto axis", "[math][vector3]") {
    constexpr Vector3 v{3.0f, 4.0f, 0.0f};
    const Vector3 proj = v.project(Vector3::right());
    REQUIRE(proj.x == Catch::Approx(3.0f));
    REQUIRE(proj.y == Catch::Approx(0.0f));
    REQUIRE(proj.z == Catch::Approx(0.0f));
}

TEST_CASE("Vector3 - angle_to returns 90 degrees between axes", "[math][vector3]") {
    const float angle = Vector3::right().angle_to(Vector3::up());
    REQUIRE(angle == Catch::Approx(star::math::Constantsf::half_pi).margin(1e-5f));
}

TEST_CASE("Vector3 - abs returns component-wise absolute value", "[math][vector3]") {
    REQUIRE(Vector3{-1.0f, -2.0f, 3.0f}.abs() == Vector3{1.0f, 2.0f, 3.0f});
}

TEST_CASE("Vector3 - index access via data union", "[math][vector3]") {
    Vector3 v{1.0f, 2.0f, 3.0f};
    REQUIRE(v[0] == 1.0f);
    REQUIRE(v[1] == 2.0f);
    REQUIRE(v[2] == 3.0f);
}
