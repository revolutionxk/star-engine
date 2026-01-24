#pragma once
#include <cmath>

#include "quaternion.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

#undef far
#undef near

namespace star::math {
    template<std::floating_point T>
    struct alignas(16) Matrix4T {
        union {
            T m[16];
            Vector4T<T> columns[4];
        };

        constexpr Matrix4T() : m{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1} {}

        constexpr explicit Matrix4T(T diagonal)
            : m{diagonal, 0, 0, 0, 0, diagonal, 0, 0, 0, 0, diagonal, 0, 0, 0, 0, diagonal} {}

        constexpr Matrix4T(T m00, T m01, T m02, T m03, T m10, T m11, T m12, T m13, T m20, T m21, T m22, T m23, T m30,
                           T m31, T m32, T m33)
            : m{m00, m10, m20, m30, m01, m11, m21, m31, m02, m12, m22, m32, m03, m13, m23, m33} {}

        static constexpr Matrix4T identity() {
            return Matrix4T();
        }

        static constexpr Matrix4T zero() {
            return Matrix4T(0);
        }

        constexpr T& operator()(u32 col, u32 row) {
            return m[col * 4 + row];
        }

        constexpr const T& operator()(u32 col, u32 row) const {
            return m[col * 4 + row];
        }

        constexpr Vector4T<T>& operator[](u32 col) {
            return columns[col];
        }

        constexpr const Vector4T<T>& operator[](u32 col) const {
            return columns[col];
        }

        Matrix4T operator*(const Matrix4T& other) const {
            Matrix4T result;
            for (u32 col = 0; col < 4; ++col) {
                for (u32 row = 0; row < 4; ++row) {
                    T sum = 0;
                    for (u32 i = 0; i < 4; ++i) {
                        sum += (*this)(i, row) * other(col, i);
                    }
                    result(col, row) = sum;
                }
            }
            return result;
        }

        Vector4T<T> operator*(const Vector4T<T>& vec) const {
            return {m[0] * vec.x + m[4] * vec.y + m[8] * vec.z + m[12] * vec.w,
                    m[1] * vec.x + m[5] * vec.y + m[9] * vec.z + m[13] * vec.w,
                    m[2] * vec.x + m[6] * vec.y + m[10] * vec.z + m[14] * vec.w,
                    m[3] * vec.x + m[7] * vec.y + m[11] * vec.z + m[15] * vec.w};
        }

        Vector3T<T> transform_point(const Vector3T<T>& point) const {
            Vector4T<T> result = (*this) * Vector4T<T>(point.x, point.y, point.z, T(1));
            return {result.x / result.w, result.y / result.w, result.z / result.w};
        }

        Vector3T<T> transform_direction(const Vector3T<T>& dir) const {
            return {m[0] * dir.x + m[4] * dir.y + m[8] * dir.z, m[1] * dir.x + m[5] * dir.y + m[9] * dir.z,
                    m[2] * dir.x + m[6] * dir.y + m[10] * dir.z};
        }

        static Matrix4T from_quaternion(const QuaternionT<T>& q) {
            Matrix4T result;
            T xx = q.x * q.x;
            T yy = q.y * q.y;
            T zz = q.z * q.z;
            T xy = q.x * q.y;
            T xz = q.x * q.z;
            T yz = q.y * q.z;
            T wx = q.w * q.x;
            T wy = q.w * q.y;
            T wz = q.w * q.z;

            result(0, 0) = T(1) - T(2) * (yy + zz);
            result(0, 1) = T(2) * (xy - wz);
            result(0, 2) = T(2) * (xz + wy);

            result(1, 0) = T(2) * (xy + wz);
            result(1, 1) = T(1) - T(2) * (xx + zz);
            result(1, 2) = T(2) * (yz - wx);

            result(2, 0) = T(2) * (xz - wy);
            result(2, 1) = T(2) * (yz + wx);
            result(2, 2) = T(1) - T(2) * (xx + yy);

            return result;
        }

        static Matrix4T translate(const Vector3T<T>& translation) {
            Matrix4T result;
            result(3, 0) = translation.x;
            result(3, 1) = translation.y;
            result(3, 2) = translation.z;
            return result;
        }

        static Matrix4T scale(const Vector3T<T>& scale) {
            Matrix4T result;
            result(0, 0) = scale.x;
            result(1, 1) = scale.y;
            result(2, 2) = scale.z;
            return result;
        }

        static Matrix4T rotate_x(T angle) {
            T c = std::cos(angle);
            T s = std::sin(angle);
            Matrix4T result;
            result(1, 1) = c;
            result(2, 1) = -s;
            result(1, 2) = s;
            result(2, 2) = c;
            return result;
        }

        static Matrix4T rotate_y(T angle) {
            T c = std::cos(angle);
            T s = std::sin(angle);
            Matrix4T result;
            result(0, 0) = c;
            result(2, 0) = s;
            result(0, 2) = -s;
            result(2, 2) = c;
            return result;
        }

        static Matrix4T rotate_z(T angle) {
            T c = std::cos(angle);
            T s = std::sin(angle);
            Matrix4T result;
            result(0, 0) = c;
            result(1, 0) = -s;
            result(0, 1) = s;
            result(1, 1) = c;
            return result;
        }

        static Matrix4T rotate(const Vector3T<T>& axis, T angle) {
            T c = std::cos(angle);
            T s = std::sin(angle);
            T t = T(1) - c;

            Vector3T<T> a = axis.normalized();

            Matrix4T result;
            result(0, 0) = t * a.x * a.x + c;
            result(0, 1) = t * a.x * a.y + s * a.z;
            result(0, 2) = t * a.x * a.z - s * a.y;

            result(1, 0) = t * a.x * a.y - s * a.z;
            result(1, 1) = t * a.y * a.y + c;
            result(1, 2) = t * a.y * a.z + s * a.x;

            result(2, 0) = t * a.x * a.z + s * a.y;
            result(2, 1) = t * a.y * a.z - s * a.x;
            result(2, 2) = t * a.z * a.z + c;

            return result;
        }

        static Matrix4T look_at(const Vector3T<T>& eye, const Vector3T<T>& center, const Vector3T<T>& up) {
            Vector3T<T> f = (center - eye).normalized();
            Vector3T<T> s = f.cross(up).normalized();
            Vector3T<T> u = s.cross(f);

            Matrix4T result;
            result(0, 0) = s.x;
            result(1, 0) = s.y;
            result(2, 0) = s.z;
            result(0, 1) = u.x;
            result(1, 1) = u.y;
            result(2, 1) = u.z;
            result(0, 2) = -f.x;
            result(1, 2) = -f.y;
            result(2, 2) = -f.z;
            result(3, 0) = -s.dot(eye);
            result(3, 1) = -u.dot(eye);
            result(3, 2) = f.dot(eye);

            return result;
        }

        static Matrix4T perspective(T fov_y, T aspect, T near, T far) {
            T tan_half_fov = std::tan(fov_y / T(2));

            Matrix4T result(0);
            result(0, 0) = T(1) / (aspect * tan_half_fov);
            result(1, 1) = T(1) / tan_half_fov;
            result(2, 2) = -(far + near) / (far - near);
            result(2, 3) = -T(1);
            result(3, 2) = -(T(2) * far * near) / (far - near);

            return result;
        }

        static Matrix4T orthographic(T left, T right, T bottom, T top, T near, T far) {
            Matrix4T result;
            result(0, 0) = T(2) / (right - left);
            result(1, 1) = T(2) / (top - bottom);
            result(2, 2) = -T(2) / (far - near);
            result(3, 0) = -(right + left) / (right - left);
            result(3, 1) = -(top + bottom) / (top - bottom);
            result(3, 2) = -(far + near) / (far - near);

            return result;
        }

        [[nodiscard]] Matrix4T transposed() const {
            Matrix4T result;
            for (u32 i = 0; i < 4; ++i) {
                for (u32 j = 0; j < 4; ++j) {
                    result(i, j) = (*this)(j, i);
                }
            }
            return result;
        }

        [[nodiscard]] T determinant() const {
            T a0 = m[0] * m[5] - m[1] * m[4];
            T a1 = m[0] * m[6] - m[2] * m[4];
            T a2 = m[0] * m[7] - m[3] * m[4];
            T a3 = m[1] * m[6] - m[2] * m[5];
            T a4 = m[1] * m[7] - m[3] * m[5];
            T a5 = m[2] * m[7] - m[3] * m[6];
            T b0 = m[8] * m[13] - m[9] * m[12];
            T b1 = m[8] * m[14] - m[10] * m[12];
            T b2 = m[8] * m[15] - m[11] * m[12];
            T b3 = m[9] * m[14] - m[10] * m[13];
            T b4 = m[9] * m[15] - m[11] * m[13];
            T b5 = m[10] * m[15] - m[11] * m[14];

            return a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
        }

        [[nodiscard]] Matrix4T inversed() const {
            Matrix4T result;

            T a0 = m[0] * m[5] - m[1] * m[4];
            T a1 = m[0] * m[6] - m[2] * m[4];
            T a2 = m[0] * m[7] - m[3] * m[4];
            T a3 = m[1] * m[6] - m[2] * m[5];
            T a4 = m[1] * m[7] - m[3] * m[5];
            T a5 = m[2] * m[7] - m[3] * m[6];
            T b0 = m[8] * m[13] - m[9] * m[12];
            T b1 = m[8] * m[14] - m[10] * m[12];
            T b2 = m[8] * m[15] - m[11] * m[12];
            T b3 = m[9] * m[14] - m[10] * m[13];
            T b4 = m[9] * m[15] - m[11] * m[13];
            T b5 = m[10] * m[15] - m[11] * m[14];

            T det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;

            if (std::abs(det) < T(1e-8)) {
                return identity();
            }

            Matrix4T inv;
            inv.m[0] = m[5] * b5 - m[6] * b4 + m[7] * b3;
            inv.m[1] = -m[1] * b5 + m[2] * b4 - m[3] * b3;
            inv.m[2] = m[13] * a5 - m[14] * a4 + m[15] * a3;
            inv.m[3] = -m[9] * a5 + m[10] * a4 - m[11] * a3;
            inv.m[4] = -m[4] * b5 + m[6] * b2 - m[7] * b1;
            inv.m[5] = m[0] * b5 - m[2] * b2 + m[3] * b1;
            inv.m[6] = -m[12] * a5 + m[14] * a2 - m[15] * a1;
            inv.m[7] = m[8] * a5 - m[10] * a2 + m[11] * a1;
            inv.m[8] = m[4] * b4 - m[5] * b2 + m[7] * b0;
            inv.m[9] = -m[0] * b4 + m[1] * b2 - m[3] * b0;
            inv.m[10] = m[12] * a4 - m[13] * a2 + m[15] * a0;
            inv.m[11] = -m[8] * a4 + m[9] * a2 - m[11] * a0;
            inv.m[12] = -m[4] * b3 + m[5] * b1 - m[6] * b0;
            inv.m[13] = m[0] * b3 - m[1] * b1 + m[2] * b0;
            inv.m[14] = -m[12] * a3 + m[13] * a1 - m[14] * a0;
            inv.m[15] = m[8] * a3 - m[9] * a1 + m[10] * a0;

            T inv_det = T(1) / det;
            for (int i = 0; i < 16; ++i) {
                result.m[i] = inv.m[i] * inv_det;
            }

            return result;
        }

        static Matrix4T inverse(const Matrix4T& mat) {
            return mat.inversed();
        }

        [[nodiscard]] const T* data() const {
            return m;
        }

        [[nodiscard]] T* data() {
            return m;
        }
    };

    using Matrix4 = Matrix4T<f32>;
    using Matrix4d = Matrix4T<f64>;

    template<std::floating_point T>
    struct Matrix3T {
        T m[9];

        constexpr Matrix3T() : m{1, 0, 0, 0, 1, 0, 0, 0, 1} {}

        static constexpr Matrix3T identity() {
            return Matrix3T();
        }

        // TODO: Add full 3x3 matrix operations
        // ;( I'M TOO LAZY, MAYBE I'LL DO IT LATER
    };

    using Matrix3 = Matrix3T<f32>;
    using Matrix3d = Matrix3T<f64>;

} // namespace star::math
