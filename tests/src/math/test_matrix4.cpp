#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "star/math/math.hpp"

using namespace star;

TEST_CASE("Matrix4 - default construction is identity", "[math][matrix4]") {
    Matrix4 m;
    for (u32 col = 0; col < 4; ++col)
        for (u32 row = 0; row < 4; ++row)
            REQUIRE(m(col, row) == Catch::Approx(col == row ? 1.0f : 0.0f));
}

TEST_CASE("Matrix4 - zero() has all components zero", "[math][matrix4]") {
    constexpr Matrix4 m = Matrix4::zero();
    for (u32 i = 0; i < 16; ++i)
        REQUIRE(m.m[i] == Catch::Approx(0.0f));
}

TEST_CASE("Matrix4 - identity * identity = identity", "[math][matrix4]") {
    Matrix4 result = Matrix4::identity() * Matrix4::identity();
    for (u32 col = 0; col < 4; ++col)
        for (u32 row = 0; row < 4; ++row)
            REQUIRE(result(col, row) == Catch::Approx(col == row ? 1.0f : 0.0f));
}

TEST_CASE("Matrix4 - translate moves a point", "[math][matrix4]") {
    const Matrix4 t = Matrix4::translate({1.0f, 2.0f, 3.0f});
    const Vector3 p = t.transform_point({0.0f, 0.0f, 0.0f});
    REQUIRE(p.x == Catch::Approx(1.0f));
    REQUIRE(p.y == Catch::Approx(2.0f));
    REQUIRE(p.z == Catch::Approx(3.0f));
}

TEST_CASE("Matrix4 - scale multiplies components", "[math][matrix4]") {
    const Matrix4 s = Matrix4::scale({2.0f, 3.0f, 4.0f});
    const Vector3 p = s.transform_point({1.0f, 1.0f, 1.0f});
    REQUIRE(p.x == Catch::Approx(2.0f));
    REQUIRE(p.y == Catch::Approx(3.0f));
    REQUIRE(p.z == Catch::Approx(4.0f));
}

TEST_CASE("Matrix4 - translate does not affect direction vectors", "[math][matrix4]") {
    const Matrix4 t = Matrix4::translate({10.0f, 10.0f, 10.0f});
    const Vector3 d = t.transform_direction({1.0f, 0.0f, 0.0f});
    REQUIRE(d.x == Catch::Approx(1.0f));
    REQUIRE(d.y == Catch::Approx(0.0f));
    REQUIRE(d.z == Catch::Approx(0.0f));
}

TEST_CASE("Matrix4 - uniform scale preserves direction", "[math][matrix4]") {
    const Matrix4 s = Matrix4::scale({5.0f, 5.0f, 5.0f});
    const Vector3 d = s.transform_direction({1.0f, 0.0f, 0.0f});
    REQUIRE(d.x == Catch::Approx(5.0f));
}

TEST_CASE("Matrix4 - rotate_x by 90 degrees maps Y to Z", "[math][matrix4]") {
    const Matrix4 r = Matrix4::rotate_x(math::Constantsf::half_pi);
    const Vector3 v = r.transform_direction({0.0f, 1.0f, 0.0f});
    REQUIRE(v.x == Catch::Approx(0.0f).margin(1e-5f));
    REQUIRE(v.y == Catch::Approx(0.0f).margin(1e-5f));
    REQUIRE(v.z == Catch::Approx(1.0f).margin(1e-5f));
}

TEST_CASE("Matrix4 - from_quaternion(identity) is identity matrix", "[math][matrix4]") {
    Matrix4 m = Matrix4::from_quaternion(Quaternion::identity());
    for (u32 col = 0; col < 4; ++col)
        for (u32 row = 0; row < 4; ++row)
            REQUIRE(m(col, row) == Catch::Approx(col == row ? 1.0f : 0.0f).margin(1e-5f));
}

TEST_CASE("Matrix4 - to_quaternion round-trips through from_quaternion", "[math][matrix4]") {
    const Quaternion original = Quaternion::from_euler(0.3f, 0.5f, 0.7f).normalized();
    const Matrix4 mat = Matrix4::from_quaternion(original);
    const Quaternion recovered = mat.to_quaternion();

    // Either q or -q represent the same rotation
    const bool same =
        recovered == original || recovered == Quaternion{-original.x, -original.y, -original.z, -original.w};
    REQUIRE(same);
}

TEST_CASE("Matrix4 - operator* with Vector4 transforms correctly", "[math][matrix4]") {
    Matrix4 t = Matrix4::translate({5.0f, 0.0f, 0.0f});
    math::Vector4T v{0.0f, 0.0f, 0.0f, 1.0f};
    auto r = t * v;
    REQUIRE(r.x == Catch::Approx(5.0f));
    REQUIRE(r.y == Catch::Approx(0.0f));
    REQUIRE(r.z == Catch::Approx(0.0f));
    REQUIRE(r.w == Catch::Approx(1.0f));
}
