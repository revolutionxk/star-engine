#pragma once
#include <cmath>
#include <concepts>

#include "common.hpp"

namespace star::math {
    template<std::floating_point T>
    struct Vector2T {
        T x, y;

        constexpr Vector2T() : x(0), y(0) {}

        constexpr Vector2T(T x, T y) : x(x), y(y) {}

        constexpr explicit Vector2T(T scalar) : x(scalar), y(scalar) {}

        static constexpr Vector2T zero() {
            return {0, 0};
        }

        static constexpr Vector2T one() {
            return {1, 1};
        }

        static constexpr Vector2T right() {
            return {1, 0};
        }

        static constexpr Vector2T left() {
            return {-1, 0};
        }

        static constexpr Vector2T up() {
            return {0, 1};
        }

        static constexpr Vector2T down() {
            return {0, -1};
        }

        constexpr T& operator[](u32 index) {
            return (&x)[index];
        }

        constexpr const T& operator[](u32 index) const {
            return (&x)[index];
        }

        constexpr Vector2T operator+(const Vector2T& other) const {
            return {x + other.x, y + other.y};
        }

        constexpr Vector2T operator-(const Vector2T& other) const {
            return {x - other.x, y - other.y};
        }

        constexpr Vector2T operator*(T scalar) const {
            return {x * scalar, y * scalar};
        }

        constexpr Vector2T operator*(const Vector2T& other) const {
            return {x * other.x, y * other.y};
        }

        constexpr Vector2T operator/(T scalar) const {
            return {x / scalar, y / scalar};
        }

        constexpr Vector2T operator/(const Vector2T& other) const {
            return {x / other.x, y / other.y};
        }

        constexpr Vector2T operator-() const {
            return {-x, -y};
        }

        constexpr Vector2T& operator+=(const Vector2T& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        constexpr Vector2T& operator-=(const Vector2T& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        constexpr Vector2T& operator*=(T scalar) {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        constexpr Vector2T& operator*=(const Vector2T& other) {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        constexpr Vector2T& operator/=(T scalar) {
            x /= scalar;
            y /= scalar;
            return *this;
        }

        constexpr Vector2T& operator/=(const Vector2T& other) {
            x /= other.x;
            y /= other.y;
            return *this;
        }

        constexpr bool operator==(const Vector2T& other) const {
            return approximately(x, other.x) && approximately(y, other.y);
        }

        constexpr bool operator!=(const Vector2T& other) const {
            return !(*this == other);
        }

        [[nodiscard]] constexpr T length_squared() const {
            return x * x + y * y;
        }

        [[nodiscard]] T length() const {
            return std::sqrt(length_squared());
        }

        [[nodiscard]] T distance_to(const Vector2T& other) const {
            return (*this - other).length();
        }

        [[nodiscard]] constexpr T distance_squared_to(const Vector2T& other) const {
            return (*this - other).length_squared();
        }

        [[nodiscard]] Vector2T normalized() const {
            T len = length();
            return len > Constants<T>::epsilon ? *this / len : zero();
        }

        void normalize() {
            T len = length();
            if (len > Constants<T>::epsilon) {
                x /= len;
                y /= len;
            }
        }

        [[nodiscard]] constexpr T dot(const Vector2T& other) const {
            return x * other.x + y * other.y;
        }

        [[nodiscard]] constexpr T cross(const Vector2T& other) const {
            return x * other.y - y * other.x;
        }

        [[nodiscard]] Vector2T lerp(const Vector2T& other, T t) const {
            return {math::lerp(x, other.x, t), math::lerp(y, other.y, t)};
        }

        [[nodiscard]] constexpr Vector2T reflect(const Vector2T& normal) const {
            return *this - normal * (T(2) * dot(normal));
        }

        [[nodiscard]] Vector2T rotated(T angle) const {
            T cos_a = std::cos(angle);
            T sin_a = std::sin(angle);
            return {x * cos_a - y * sin_a, x * sin_a + y * cos_a};
        }

        [[nodiscard]] T angle() const {
            return std::atan2(y, x);
        }

        [[nodiscard]] T angle_to(const Vector2T& other) const {
            return std::atan2(cross(other), dot(other));
        }

        [[nodiscard]] constexpr Vector2T perpendicular() const {
            return {-y, x};
        }

        [[nodiscard]] constexpr Vector2T abs() const {
            return {std::abs(x), std::abs(y)};
        }

        [[nodiscard]] constexpr Vector2T clamp(const Vector2T& min, const Vector2T& max) const {
            return {math::clamp(x, min.x, max.x), math::clamp(y, min.y, max.y)};
        }

        [[nodiscard]] bool is_normalized() const {
            return approximately(length_squared(), T(1));
        }

        [[nodiscard]] bool is_zero() const {
            return approximately(x, T(0)) && approximately(y, T(0));
        }
    };

    template<std::floating_point T>
    constexpr Vector2T<T> operator*(T scalar, const Vector2T<T>& vec) {
        return vec * scalar;
    }

    using Vector2 = Vector2T<f32>;
    using Vector2d = Vector2T<f64>;

} // namespace star::math
