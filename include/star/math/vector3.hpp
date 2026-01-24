#pragma once
#include <cmath>
#include <concepts>

#include "common.hpp"

namespace star::math {
    template<std::floating_point T>
    struct Vector3T {
        union {
            struct {
                T x, y, z;
            };

            T data[3];
        };

        constexpr Vector3T() : x(0), y(0), z(0) {}

        constexpr Vector3T(T x, T y, T z) : x(x), y(y), z(z) {}

        constexpr explicit Vector3T(T scalar) : x(scalar), y(scalar), z(scalar) {}

        static constexpr Vector3T zero() {
            return {0, 0, 0};
        }

        static constexpr Vector3T one() {
            return {1, 1, 1};
        }

        static constexpr Vector3T right() {
            return {1, 0, 0};
        }

        static constexpr Vector3T left() {
            return {-1, 0, 0};
        }

        static constexpr Vector3T up() {
            return {0, 1, 0};
        }

        static constexpr Vector3T down() {
            return {0, -1, 0};
        }

        static constexpr Vector3T forward() {
            return {0, 0, 1};
        }

        static constexpr Vector3T back() {
            return {0, 0, -1};
        }

        constexpr T& operator[](u32 index) {
            return data[index];
        }

        constexpr const T& operator[](u32 index) const {
            return data[index];
        }

        constexpr Vector3T operator+(const Vector3T& other) const {
            return {x + other.x, y + other.y, z + other.z};
        }

        constexpr Vector3T operator-(const Vector3T& other) const {
            return {x - other.x, y - other.y, z - other.z};
        }

        constexpr Vector3T operator*(T scalar) const {
            return {x * scalar, y * scalar, z * scalar};
        }

        constexpr Vector3T operator*(const Vector3T& other) const {
            return {x * other.x, y * other.y, z * other.z};
        }

        constexpr Vector3T operator/(T scalar) const {
            return {x / scalar, y / scalar, z / scalar};
        }

        constexpr Vector3T operator/(const Vector3T& other) const {
            return {x / other.x, y / other.y, z / other.z};
        }

        constexpr Vector3T operator-() const {
            return {-x, -y, -z};
        }

        constexpr Vector3T& operator=(const float* other) {
            x = other[0];
            y = other[1];
            z = other[2];

            return *this;
        }

        constexpr Vector3T& operator=(const Vector3T& other) {
            x = other.x;
            y = other.y;
            z = other.z;

            return *this;
        }

        constexpr Vector3T& operator=(const std::array<float, 3> other) {
            x = other[0];
            y = other[1];
            z = other[2];

            return *this;
        }

        constexpr Vector3T& operator+=(const Vector3T& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        constexpr Vector3T& operator-=(const Vector3T& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        constexpr Vector3T& operator*=(T scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }

        constexpr Vector3T& operator*=(const Vector3T& other) {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            return *this;
        }

        constexpr Vector3T& operator/=(T scalar) {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            return *this;
        }

        constexpr Vector3T& operator/=(const Vector3T& other) {
            x /= other.x;
            y /= other.y;
            z /= other.z;
            return *this;
        }

        constexpr bool operator==(const Vector3T& other) const {
            return approximately(x, other.x) && approximately(y, other.y) && approximately(z, other.z);
        }

        constexpr bool operator!=(const Vector3T& other) const {
            return !(*this == other);
        }

        [[nodiscard]] constexpr T length_squared() const {
            return x * x + y * y + z * z;
        }

        [[nodiscard]] T length() const {
            return std::sqrt(length_squared());
        }

        [[nodiscard]] T distance_to(const Vector3T& other) const {
            return (*this - other).length();
        }

        [[nodiscard]] constexpr T distance_squared_to(const Vector3T& other) const {
            return (*this - other).length_squared();
        }

        [[nodiscard]] Vector3T normalized() const {
            T len = length();
            return len > Constants<T>::epsilon ? *this / len : zero();
        }

        void normalize() {
            T len = length();
            if (len > Constants<T>::epsilon) {
                x /= len;
                y /= len;
                z /= len;
            }
        }

        [[nodiscard]] constexpr T dot(const Vector3T& other) const {
            return x * other.x + y * other.y + z * other.z;
        }

        [[nodiscard]] constexpr Vector3T cross(const Vector3T& other) const {
            return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
        }

        [[nodiscard]] Vector3T lerp(const Vector3T& other, T t) const {
            return {math::lerp(x, other.x, t), math::lerp(y, other.y, t), math::lerp(z, other.z, t)};
        }

        [[nodiscard]] constexpr Vector3T reflect(const Vector3T& normal) const {
            return *this - normal * (T(2) * dot(normal));
        }

        [[nodiscard]] Vector3T project(const Vector3T& onto) const {
            T d = onto.length_squared();
            if (d > Constants<T>::epsilon) {
                return onto * (dot(onto) / d);
            }
            return zero();
        }

        [[nodiscard]] T angle_to(const Vector3T& other) const {
            T d = std::sqrt(length_squared() * other.length_squared());
            if (d < Constants<T>::epsilon)
                return T(0);
            return std::acos(clamp(dot(other) / d, T(-1), T(1)));
        }

        [[nodiscard]] constexpr Vector3T abs() const {
            return {std::abs(x), std::abs(y), std::abs(z)};
        }

        [[nodiscard]] constexpr Vector3T clamp(const Vector3T& min, const Vector3T& max) const {
            return {math::clamp(x, min.x, max.x), math::clamp(y, min.y, max.y), math::clamp(z, min.z, max.z)};
        }

        [[nodiscard]] bool is_normalized() const {
            return approximately(length_squared(), T(1));
        }

        [[nodiscard]] bool is_zero() const {
            return approximately(x, T(0)) && approximately(y, T(0)) && approximately(z, T(0));
        }

        [[nodiscard]] constexpr Vector3T xyz() const {
            return *this;
        }

        [[nodiscard]] constexpr Vector3T xzy() const {
            return {x, z, y};
        }

        [[nodiscard]] constexpr Vector3T yxz() const {
            return {y, x, z};
        }
    };

    template<std::floating_point T>
    constexpr Vector3T<T> operator*(T scalar, const Vector3T<T>& vec) {
        return vec * scalar;
    }

    using Vector3 = Vector3T<f32>;
    using Vector3d = Vector3T<f64>;

} // namespace star::math
