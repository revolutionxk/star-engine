#pragma once
#include <cmath>

#include "matrix.hpp"
#include "vector3.hpp"

namespace star::math {
    template<std::floating_point T>
    struct QuaternionT {
        T x, y, z, w;

        constexpr QuaternionT() : x(0), y(0), z(0), w(1) {}

        constexpr QuaternionT(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

        constexpr QuaternionT(const Vector3T<T>& axis, T angle) {
            T half_angle = angle * T(0.5);
            T s = std::sin(half_angle);
            w = std::cos(half_angle);
            x = axis.x * s;
            y = axis.y * s;
            z = axis.z * s;
        }

        static constexpr QuaternionT identity() {
            return {0, 0, 0, 1};
        }

        static QuaternionT from_euler(T pitch, T yaw, T roll) {
            T cy = std::cos(yaw * T(0.5));
            T sy = std::sin(yaw * T(0.5));
            T cp = std::cos(pitch * T(0.5));
            T sp = std::sin(pitch * T(0.5));
            T cr = std::cos(roll * T(0.5));
            T sr = std::sin(roll * T(0.5));

            return {sr * cp * cy - cr * sp * sy, cr * sp * cy + sr * cp * sy, cr * cp * sy - sr * sp * cy,
                    cr * cp * cy + sr * sp * sy};
        }

        static QuaternionT from_euler(const Vector3T<T>& euler) {
            return from_euler(euler.x, euler.y, euler.z);
        }

        QuaternionT operator*(const QuaternionT& other) const {
            return {w * other.x + x * other.w + y * other.z - z * other.y,
                    w * other.y + y * other.w + z * other.x - x * other.z,
                    w * other.z + z * other.w + x * other.y - y * other.x,
                    w * other.w - x * other.x - y * other.y - z * other.z};
        }

        Vector3T<T> operator*(const Vector3T<T>& vec) const {
            Vector3T<T> u(x, y, z);
            T s = w;

            return u * (T(2) * u.dot(vec)) + vec * (s * s - u.dot(u)) + u.cross(vec) * (T(2) * s);
        }

        QuaternionT operator*(T scalar) const {
            return {x * scalar, y * scalar, z * scalar, w * scalar};
        }

        QuaternionT operator+(const QuaternionT& other) const {
            return {x + other.x, y + other.y, z + other.z, w + other.w};
        }

        QuaternionT operator-() const {
            return {-x, -y, -z, -w};
        }

        bool operator==(const QuaternionT& other) const {
            return approximately(x, other.x) && approximately(y, other.y) && approximately(z, other.z) &&
                   approximately(w, other.w);
        }

        [[nodiscard]] constexpr T length_squared() const {
            return x * x + y * y + z * z + w * w;
        }

        [[nodiscard]] T length() const {
            return std::sqrt(length_squared());
        }

        [[nodiscard]] QuaternionT normalized() const {
            T len = length();
            return len > Constants<T>::epsilon ? (*this) * (T(1) / len) : identity();
        }

        void normalize() {
            T len = length();
            if (len > Constants<T>::epsilon) {
                T inv = T(1) / len;
                x *= inv;
                y *= inv;
                z *= inv;
                w *= inv;
            }
        }

        [[nodiscard]] constexpr QuaternionT conjugate() const {
            return {-x, -y, -z, w};
        }

        [[nodiscard]] QuaternionT inverse() const {
            T len_sq = length_squared();
            if (len_sq > Constants<T>::epsilon) {
                return conjugate() * (T(1) / len_sq);
            }
            return identity();
        }

        [[nodiscard]] constexpr T dot(const QuaternionT& other) const {
            return x * other.x + y * other.y + z * other.z + w * other.w;
        }

        [[nodiscard]] QuaternionT slerp(const QuaternionT& other, T t) const {
            QuaternionT q = other;
            T cos_theta = dot(other);

            // Take shorter path
            if (cos_theta < T(0)) {
                q = -q;
                cos_theta = -cos_theta;
            }

            // Linear interpolation for small angles
            if (cos_theta > T(0.9995)) {
                return (*this) * (T(1) - t) + q * t;
            }

            T theta = std::acos(cos_theta);
            T sin_theta = std::sin(theta);

            T a = std::sin((T(1) - t) * theta) / sin_theta;
            T b = std::sin(t * theta) / sin_theta;

            return (*this) * a + q * b;
        }

        [[nodiscard]] Vector3T<T> to_euler() const {
            // Roll (x-axis rotation)
            T sinr_cosp = T(2) * (w * x + y * z);
            T cosr_cosp = T(1) - T(2) * (x * x + y * y);
            T roll = std::atan2(sinr_cosp, cosr_cosp);

            // Pitch (y-axis rotation)
            T sinp = T(2) * (w * y - z * x);
            T pitch;
            if (std::abs(sinp) >= T(1))
                pitch = std::copysign(Constants<T>::half_pi, sinp);
            else
                pitch = std::asin(sinp);

            // Yaw (z-axis rotation)
            T siny_cosp = T(2) * (w * z + x * y);
            T cosy_cosp = T(1) - T(2) * (y * y + z * z);
            T yaw = std::atan2(siny_cosp, cosy_cosp);

            return {pitch, yaw, roll};
        }

        [[nodiscard]] Matrix4T<T> to_matrix() const {
            T xx = x * x, yy = y * y, zz = z * z;
            T xy = x * y, xz = x * z, yz = y * z;
            T wx = w * x, wy = w * y, wz = w * z;

            return Matrix4T<T>(T(1) - T(2) * (yy + zz), T(2) * (xy + wz), T(2) * (xz - wy), 0, T(2) * (xy - wz),
                               T(1) - T(2) * (xx + zz), T(2) * (yz + wx), 0, T(2) * (xz + wy), T(2) * (yz - wx),
                               T(1) - T(2) * (xx + yy), 0, 0, 0, 0, 1);
        }

        [[nodiscard]] bool is_normalized() const {
            return approximately(length_squared(), T(1));
        }
    };

    using Quaternion = QuaternionT<f32>;
    using Quaterniond = QuaternionT<f64>;

} // namespace star::math
