#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "star/math/math.hpp"

using Vector2 = math::Vector2T<float>;

TEST_CASE("Vector2 - default construction is zero", "[math][vector2]") {
    constexpr Vector2 v;
    REQUIRE(v.x == 0.0f);
    REQUIRE(v.y == 0.0f);
}

TEST_CASE("Vector2 - component construction", "[math][vector2]") {
    constexpr Vector2 v{3.0f, 4.0f};
    REQUIRE(v.x == 3.0f);
    REQUIRE(v.y == 4.0f);
}

TEST_CASE("Vector2 - scalar broadcast construction", "[math][vector2]") {
    constexpr Vector2 v{2.0f};
    REQUIRE(v.x == 2.0f);
    REQUIRE(v.y == 2.0f);
}

TEST_CASE("Vector2 - static factories", "[math][vector2]") {
    REQUIRE(Vector2::zero() == Vector2{0.0f, 0.0f});
    REQUIRE(Vector2::one() == Vector2{1.0f, 1.0f});
    REQUIRE(Vector2::right() == Vector2{1.0f, 0.0f});
    REQUIRE(Vector2::left() == Vector2{-1.0f, 0.0f});
    REQUIRE(Vector2::up() == Vector2{0.0f, 1.0f});
    REQUIRE(Vector2::down() == Vector2{0.0f, -1.0f});
}

TEST_CASE("Vector2 - arithmetic operators", "[math][vector2]") {
    constexpr Vector2 a{1.0f, 2.0f};
    constexpr Vector2 b{3.0f, 4.0f};

    SECTION("addition") {
        REQUIRE(a + b == Vector2{4.0f, 6.0f});
    }
    SECTION("subtraction") {
        REQUIRE(b - a == Vector2{2.0f, 2.0f});
    }
    SECTION("scalar multiply") {
        REQUIRE(a * 2.0f == Vector2{2.0f, 4.0f});
    }
    SECTION("component multiply") {
        REQUIRE(a * b == Vector2{3.0f, 8.0f});
    }
    SECTION("scalar divide") {
        REQUIRE(b / 2.0f == Vector2{1.5f, 2.0f});
    }
    SECTION("component divide") {
        REQUIRE(Vector2{4.0f, 6.0f} / Vector2{2.0f, 3.0f} == Vector2{2.0f, 2.0f});
    }
    SECTION("negate") {
        REQUIRE(-a == Vector2{-1.0f, -2.0f});
    }
}

TEST_CASE("Vector2 - compound assignment operators", "[math][vector2]") {
    Vector2 v{1.0f, 2.0f};

    v += Vector2{1.0f, 1.0f};
    REQUIRE(v == Vector2{2.0f, 3.0f});

    v -= Vector2{1.0f, 1.0f};
    REQUIRE(v == Vector2{1.0f, 2.0f});

    v *= 3.0f;
    REQUIRE(v == Vector2{3.0f, 6.0f});

    v /= 3.0f;
    REQUIRE(v == Vector2{1.0f, 2.0f});
}

TEST_CASE("Vector2 - length squared", "[math][vector2]") {
    REQUIRE(Vector2{3.0f, 4.0f}.length_squared() == Catch::Approx(25.0f));
    REQUIRE(Vector2::zero().length_squared() == Catch::Approx(0.0f));
}

TEST_CASE("Vector2 - length", "[math][vector2]") {
    REQUIRE(Vector2{3.0f, 4.0f}.length() == Catch::Approx(5.0f));
    REQUIRE(Vector2{1.0f, 0.0f}.length() == Catch::Approx(1.0f));
}

TEST_CASE("Vector2 - normalized returns unit vector", "[math][vector2]") {
    Vector2 v{3.0f, 4.0f};
    Vector2 n = v.normalized();
    REQUIRE(n.length() == Catch::Approx(1.0f).margin(1e-5f));
}

TEST_CASE("Vector2 - normalizing zero vector returns zero", "[math][vector2]") {
    REQUIRE(Vector2::zero().normalized() == Vector2::zero());
}

TEST_CASE("Vector2 - normalize in-place", "[math][vector2]") {
    Vector2 v{0.0f, 5.0f};
    v.normalize();
    REQUIRE(v.length() == Catch::Approx(1.0f).margin(1e-5f));
}

TEST_CASE("Vector2 - distance_to", "[math][vector2]") {
    REQUIRE(Vector2{0.0f, 0.0f}.distance_to({3.0f, 4.0f}) == Catch::Approx(5.0f));
    REQUIRE(Vector2{1.0f, 1.0f}.distance_to({1.0f, 1.0f}) == Catch::Approx(0.0f));
}

TEST_CASE("Vector2 - distance_squared_to", "[math][vector2]") {
    REQUIRE(Vector2{0.0f, 0.0f}.distance_squared_to({3.0f, 4.0f}) == Catch::Approx(25.0f));
}

TEST_CASE("Vector2 - index access", "[math][vector2]") {
    Vector2 v{5.0f, 7.0f};
    REQUIRE(v[0] == 5.0f);
    REQUIRE(v[1] == 7.0f);
}
