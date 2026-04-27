#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "star/math/math.hpp"

using namespace star;

TEST_CASE("Quaternion - identity has correct components", "[math][quaternion]") {
    constexpr Quaternion q = Quaternion::identity();
    REQUIRE(q.x == Catch::Approx(0.0f));
    REQUIRE(q.y == Catch::Approx(0.0f));
    REQUIRE(q.z == Catch::Approx(0.0f));
    REQUIRE(q.w == Catch::Approx(1.0f));
}

TEST_CASE("Quaternion - identity is unit length", "[math][quaternion]") {
    REQUIRE(Quaternion::identity().length() == Catch::Approx(1.0f));
}

TEST_CASE("Quaternion - identity rotates vector unchanged", "[math][quaternion]") {
    constexpr Quaternion id = Quaternion::identity();
    constexpr Vector3 v{1.0f, 2.0f, 3.0f};
    const Vector3 r = id * v;
    REQUIRE(r.x == Catch::Approx(v.x).margin(1e-5f));
    REQUIRE(r.y == Catch::Approx(v.y).margin(1e-5f));
    REQUIRE(r.z == Catch::Approx(v.z).margin(1e-5f));
}

TEST_CASE("Quaternion - from_euler zero is identity", "[math][quaternion]") {
    Quaternion q = Quaternion::from_euler(0.0f, 0.0f, 0.0f);
    REQUIRE(q == Quaternion::identity());
}

TEST_CASE("Quaternion - from_euler vec overload matches scalar overload", "[math][quaternion]") {
    Quaternion a = Quaternion::from_euler(0.1f, 0.2f, 0.3f);
    Quaternion b = Quaternion::from_euler(Vector3{0.1f, 0.2f, 0.3f});
    REQUIRE(a == b);
}

TEST_CASE("Quaternion - length_squared of identity is 1", "[math][quaternion]") {
    REQUIRE(Quaternion::identity().length_squared() == Catch::Approx(1.0f));
}

TEST_CASE("Quaternion - normalized returns unit quaternion", "[math][quaternion]") {
    constexpr Quaternion q{1.0f, 1.0f, 1.0f, 1.0f};
    const Quaternion n = q.normalized();
    REQUIRE(n.length() == Catch::Approx(1.0f).margin(1e-5f));
}

TEST_CASE("Quaternion - normalizing zero quaternion returns identity", "[math][quaternion]") {
    constexpr Quaternion zero{0.0f, 0.0f, 0.0f, 0.0f};
    const Quaternion n = zero.normalized();
    REQUIRE(n == Quaternion::identity());
}

TEST_CASE("Quaternion - conjugate negates xyz, preserves w", "[math][quaternion]") {
    constexpr Quaternion q{0.5f, 0.5f, 0.5f, 0.5f};
    constexpr Quaternion c = q.conjugate();
    REQUIRE(c.x == Catch::Approx(-0.5f));
    REQUIRE(c.y == Catch::Approx(-0.5f));
    REQUIRE(c.z == Catch::Approx(-0.5f));
    REQUIRE(c.w == Catch::Approx(0.5f));
}

TEST_CASE("Quaternion - q * inverse(q) is identity", "[math][quaternion]") {
    const Quaternion q = Quaternion::from_euler(0.3f, 0.5f, 0.7f).normalized();
    const Quaternion inv = q.inverse();
    const Quaternion result = q * inv;
    REQUIRE(result == Quaternion::identity());
}

TEST_CASE("Quaternion - multiplying by identity is idempotent", "[math][quaternion]") {
    constexpr Quaternion id = Quaternion::identity();
    const Quaternion q = Quaternion::from_euler(0.1f, 0.2f, 0.3f);
    REQUIRE(q * id == q);
    REQUIRE(id * q == q);
}

TEST_CASE("Quaternion - dot product of identity with itself is 1", "[math][quaternion]") {
    constexpr Quaternion id = Quaternion::identity();
    REQUIRE(id.dot(id) == Catch::Approx(1.0f));
}

TEST_CASE("Quaternion - 90-degree rotation around Y preserves vector length", "[math][quaternion]") {
    Quaternion q{Vector3::up(), math::Constantsf::half_pi};
    q = q.normalized();
    constexpr Vector3 v{1.0f, 0.0f, 0.0f};
    const Vector3 r = q * v;
    REQUIRE(r.length() == Catch::Approx(1.0f).margin(1e-4f));
}

TEST_CASE("Quaternion - rotation around Z by 180 degrees flips X", "[math][quaternion]") {
    Quaternion q{Vector3::back(), math::Constantsf::pi};
    q = q.normalized();
    constexpr Vector3 v{1.0f, 0.0f, 0.0f};
    const Vector3 r = q * v;
    REQUIRE(r.x == Catch::Approx(-1.0f).margin(1e-4f));
    REQUIRE(r.y == Catch::Approx(0.0f).margin(1e-4f));
}
