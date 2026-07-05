#pragma once
#include <algorithm>
#include <optional>

#include "vector3.hpp"

namespace star::math {
    template<std::floating_point T>
    struct AABBT {
        Vector3T<T> min;
        Vector3T<T> max;

        constexpr AABBT() : min(std::numeric_limits<T>::max()), max(std::numeric_limits<T>::lowest()) {}

        constexpr AABBT(const Vector3T<T>& min, const Vector3T<T>& max) : min(min), max(max) {}

        static constexpr AABBT from_center_extents(const Vector3T<T>& center, const Vector3T<T>& half_extents) {
            return AABBT{center - half_extents, center + half_extents};
        }

        static constexpr AABBT from_point(const Vector3T<T>& point) {
            return AABBT{point, point};
        }

        constexpr Vector3T<T> center() const {
            return (min + max) * T(0.5);
        }

        constexpr Vector3T<T> half_extents() const {
            return (max - min) * T(0.5);
        }

        constexpr Vector3T<T> extents() const {
            return max - min;
        }

        constexpr T volume() const {
            const auto e = extents();
            return e.x * e.y * e.z;
        }

        constexpr T surface_area() const {
            const auto e = extents();
            return T(2) * (e.x * e.y + e.y * e.z + e.z * e.x);
        }

        constexpr bool is_valid() const {
            return min.x <= max.x && min.y <= max.y && min.z <= max.z;
        }

        constexpr bool is_empty() const {
            return !is_valid() || min == max;
        }

        constexpr bool contains(const Vector3T<T>& point) const {
            return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y && point.z >= min.z &&
                   point.z <= max.z;
        }

        constexpr bool contains(const AABBT& other) const {
            return contains(other.min) && contains(other.max);
        }

        constexpr bool intersects(const AABBT& other) const {
            return !(max.x < other.min.x || min.x > other.max.x || max.y < other.min.y || min.y > other.max.y ||
                     max.z < other.min.z || min.z > other.max.z);
        }

        constexpr void expand(const Vector3T<T>& point) {
            min.x = std::min(min.x, point.x);
            min.y = std::min(min.y, point.y);
            min.z = std::min(min.z, point.z);

            max.x = std::max(max.x, point.x);
            max.y = std::max(max.y, point.y);
            max.z = std::max(max.z, point.z);
        }

        constexpr void expand(const AABBT& other) {
            if (!other.is_valid()) {
                return;
            }

            expand(other.min);
            expand(other.max);
        }

        constexpr void expand(T amount) {
            min -= Vector3T<T>(amount);
            max += Vector3T<T>(amount);
        }

        constexpr AABBT expanded(T amount) const {
            return AABBT{min - Vector3T<T>(amount), max + Vector3T<T>(amount)};
        }

        constexpr std::optional<AABBT> intersection(const AABBT& other) const {
            if (!intersects(other)) {
                return std::nullopt;
            }

            AABBT result;
            result.min.x = std::max(min.x, other.min.x);
            result.min.y = std::max(min.y, other.min.y);
            result.min.z = std::max(min.z, other.min.z);

            result.max.x = std::min(max.x, other.max.x);
            result.max.y = std::min(max.y, other.max.y);
            result.max.z = std::min(max.z, other.max.z);

            return result;
        }

        constexpr AABBT union_with(const AABBT& other) const {
            if (!is_valid()) {
                return other;
            }
            if (!other.is_valid()) {
                return *this;
            }

            AABBT result;
            result.min.x = std::min(min.x, other.min.x);
            result.min.y = std::min(min.y, other.min.y);
            result.min.z = std::min(min.z, other.min.z);

            result.max.x = std::max(max.x, other.max.x);
            result.max.y = std::max(max.y, other.max.y);
            result.max.z = std::max(max.z, other.max.z);

            return result;
        }

        constexpr Vector3T<T> closest_point(const Vector3T<T>& point) const {
            return Vector3T<T>{std::clamp(point.x, min.x, max.x), std::clamp(point.y, min.y, max.y),
                               std::clamp(point.z, min.z, max.z)};
        }

        constexpr T distance_squared(const Vector3T<T>& point) const {
            const auto closest = closest_point(point);
            return (point - closest).length_squared();
        }

        T distance(const Vector3T<T>& point) const {
            return std::sqrt(distance_squared(point));
        }

        constexpr Vector3T<T> corner(size_t index) const {
            return Vector3T<T>{(index & 1) ? max.x : min.x, (index & 2) ? max.y : min.y, (index & 4) ? max.z : min.z};
        }

        template<typename Matrix4T>
        AABBT transformed(const Matrix4T& matrix) const {
            if (!is_valid()) {
                return *this;
            }

            AABBT result = from_point(matrix * Vector4T<T>(min, T(1)));

            for (size_t i = 1; i < 8; ++i) {
                const auto transformed_corner = matrix * Vector4T<T>(corner(i), T(1));
                result.expand(Vector3T<T>(transformed_corner.x, transformed_corner.y, transformed_corner.z));
            }

            return result;
        }

        std::optional<T> intersect_ray(const Vector3T<T>& origin, const Vector3T<T>& direction) const {
            T tmin = T(0);
            T tmax = std::numeric_limits<T>::max();

            for (int axis = 0; axis < 3; ++axis) {
                const T o = origin[axis];
                const T d = direction[axis];
                const T lo = min[axis];
                const T hi = max[axis];

                if (d > T(-1e-9) && d < T(1e-9)) {
                    if (o < lo || o > hi)
                        return std::nullopt;
                } else {
                    const T inv = T(1) / d;
                    T t1 = (lo - o) * inv;
                    T t2 = (hi - o) * inv;
                    if (t1 > t2)
                        std::swap(t1, t2);
                    tmin = std::max(tmin, t1);
                    tmax = std::min(tmax, t2);
                    if (tmin > tmax)
                        return std::nullopt;
                }
            }
            return tmin;
        }

        constexpr bool operator==(const AABBT& other) const {
            return min == other.min && max == other.max;
        }

        constexpr bool operator!=(const AABBT& other) const {
            return !(*this == other);
        }
    };

    using AABB = AABBT<float>;
    using AABBd = AABBT<double>;
} // namespace star::math
