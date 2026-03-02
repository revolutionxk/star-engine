#pragma once

#include "star/core/types.hpp"
#include "star/math/math.hpp"

namespace star::rendering {
    struct Frustum {
        struct Plane {
            Vector3 normal;
            f32 d;

            [[nodiscard]] constexpr f32 signed_distance(const Vector3& p) const noexcept {
                return normal.x * p.x + normal.y * p.y + normal.z * p.z + d;
            }
        };

        Plane planes[6];

        [[nodiscard]] static Frustum extract(const Matrix4& vp, bool homogeneous_depth = true) noexcept;

        [[nodiscard]] bool intersects_aabb(const AABB& aabb) const noexcept;
        [[nodiscard]] bool intersects_aabb_world(const AABB& local_bounds, const Matrix4& model) const noexcept;

      private:
        static AABB transform_aabb(const AABB& local, const Matrix4& model) noexcept;
    };
} // namespace star::rendering
