#pragma once
#include <cmath>
#include <concepts>

#include "common.hpp"

namespace star::math {
    template<std::floating_point T>
    struct alignas(16) Vector4T {
        union {
            struct {
                T x, y, z, w;
            };

            struct {
                T r, g, b, a;
            };

            T data[4];
        };

        constexpr Vector4T() : x(0), y(0), z(0), w(0) {}

        constexpr Vector4T(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

        constexpr explicit Vector4T(T scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}

        constexpr Vector4T(const Vector3T<T>& xyz, T w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}

        static constexpr Vector4T zero() {
            return {0, 0, 0, 0};
        }

        static constexpr Vector4T one() {
            return {1, 1, 1, 1};
        }

        constexpr T& operator[](u32 index) {
            return data[index];
        }

        constexpr const T& operator[](u32 index) const {
            return data[index];
        }

        constexpr Vector4T operator+(const Vector4T& other) const {
            return {x + other.x, y + other.y, z + other.z, w + other.w};
        }

        constexpr Vector4T operator-(const Vector4T& other) const {
            return {x - other.x, y - other.y, z - other.z, w - other.w};
        }

        constexpr Vector4T operator*(T scalar) const {
            return {x * scalar, y * scalar, z * scalar, w * scalar};
        }

        constexpr Vector4T operator*(const Vector4T& other) const {
            return {x * other.x, y * other.y, z * other.z, w * other.w};
        }

        constexpr Vector4T operator/(T scalar) const {
            return {x / scalar, y / scalar, z / scalar, w / scalar};
        }

        constexpr Vector4T operator/(const Vector4T& other) const {
            return {x / other.x, y / other.y, z / other.z, w / other.w};
        }

        constexpr Vector4T operator-() const {
            return {-x, -y, -z, -w};
        }

        constexpr Vector4T& operator+=(const Vector4T& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
            return *this;
        }

        constexpr Vector4T& operator-=(const Vector4T& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            w -= other.w;
            return *this;
        }

        constexpr Vector4T& operator*=(T scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            w *= scalar;
            return *this;
        }

        constexpr Vector4T& operator*=(const Vector4T& other) {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            w *= other.w;
            return *this;
        }

        constexpr Vector4T& operator/=(T scalar) {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            w /= scalar;
            return *this;
        }

        constexpr bool operator==(const Vector4T& other) const {
            return approximately(x, other.x) && approximately(y, other.y) && approximately(z, other.z) &&
                   approximately(w, other.w);
        }

        constexpr bool operator!=(const Vector4T& other) const {
            return !(*this == other);
        }

        [[nodiscard]] constexpr T length_squared() const {
            return x * x + y * y + z * z + w * w;
        }

        [[nodiscard]] T length() const {
            return std::sqrt(length_squared());
        }

        [[nodiscard]] Vector4T normalized() const {
            T len = length();
            return len > Constants<T>::epsilon ? *this / len : zero();
        }

        void normalize() {
            T len = length();
            if (len > Constants<T>::epsilon) {
                x /= len;
                y /= len;
                z /= len;
                w /= len;
            }
        }

        [[nodiscard]] constexpr T dot(const Vector4T& other) const {
            return x * other.x + y * other.y + z * other.z + w * other.w;
        }

        [[nodiscard]] Vector4T lerp(const Vector4T& other, T t) const {
            return {math::lerp(x, other.x, t), math::lerp(y, other.y, t), math::lerp(z, other.z, t),
                    math::lerp(w, other.w, t)};
        }

        [[nodiscard]] constexpr Vector4T abs() const {
            return {std::abs(x), std::abs(y), std::abs(z), std::abs(w)};
        }

        [[nodiscard]] bool is_normalized() const {
            return approximately(length_squared(), T(1));
        }

        [[nodiscard]] bool is_zero() const {
            return approximately(x, T(0)) && approximately(y, T(0)) && approximately(z, T(0)) && approximately(w, T(0));
        }

        [[nodiscard]] constexpr Vector3T<T> xyz() const {
            return {x, y, z};
        }

        [[nodiscard]] constexpr Vector3T<T> rgb() const {
            return {r, g, b};
        }
    };

    template<std::floating_point T>
    constexpr Vector4T<T> operator*(T scalar, const Vector4T<T>& vec) {
        return vec * scalar;
    }

    using Vector4 = Vector4T<f32>;
    using Vector4d = Vector4T<f64>;

    using Color3 = Vector3T<f32>;
    using Color4 = Vector4T<f32>;

} // namespace star::math
