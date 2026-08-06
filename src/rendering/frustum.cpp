#include "star/rendering/frustum.hpp"

#include <cmath>

namespace star::rendering {
    namespace detail {
        Frustum::Plane make_plane(const f32 a, const f32 b, const f32 c, const f32 d) noexcept {
            const f32 inv_len = 1.0f / std::sqrt(a * a + b * b + c * c);
            return {{a * inv_len, b * inv_len, c * inv_len}, d * inv_len};
        }
    } // namespace detail

    Frustum Frustum::extract(const Matrix4& vp, const bool homogeneous_depth) noexcept {
        const f32* m = vp.m;

        Frustum f;
        f.planes[0] = detail::make_plane(m[0] + m[3], m[4] + m[7], m[8] + m[11], m[12] + m[15]); // Left
        f.planes[1] = detail::make_plane(m[3] - m[0], m[7] - m[4], m[11] - m[8], m[15] - m[12]); // Right
        f.planes[2] = detail::make_plane(m[1] + m[3], m[5] + m[7], m[9] + m[11], m[13] + m[15]); // Bottom
        f.planes[3] = detail::make_plane(m[3] - m[1], m[7] - m[5], m[11] - m[9], m[15] - m[13]); // Top

        if (homogeneous_depth)
            f.planes[4] = detail::make_plane(m[2] + m[3], m[6] + m[7], m[10] + m[11], m[14] + m[15]); // Near (OpenGL)
        else
            f.planes[4] = detail::make_plane(m[2], m[6], m[10], m[14]);                               // Near (D3D)

        f.planes[5] = detail::make_plane(m[3] - m[2], m[7] - m[6], m[11] - m[10], m[15] - m[14]);     // Far

        return f;
    }

    bool Frustum::intersects_aabb(const AABB& aabb) const noexcept {
        for (const auto& plane : planes) {
            const Vector3 pv{
                plane.normal.x >= 0.0f ? aabb.max.x : aabb.min.x,
                plane.normal.y >= 0.0f ? aabb.max.y : aabb.min.y,
                plane.normal.z >= 0.0f ? aabb.max.z : aabb.min.z,
            };
            if (plane.signed_distance(pv) < 0.0f)
                return false;
        }
        return true;
    }

    AABB Frustum::transform_aabb(const AABB& local, const Matrix4& model) noexcept {
        const Vector3 center = model.transform_point(local.center());
        const Vector3 half = local.half_extents();

        const Vector3 cx{model.columns[0].x, model.columns[0].y, model.columns[0].z};
        const Vector3 cy{model.columns[1].x, model.columns[1].y, model.columns[1].z};
        const Vector3 cz{model.columns[2].x, model.columns[2].y, model.columns[2].z};

        const Vector3 world_half{
            std::abs(cx.x) * half.x + std::abs(cy.x) * half.y + std::abs(cz.x) * half.z,
            std::abs(cx.y) * half.x + std::abs(cy.y) * half.y + std::abs(cz.y) * half.z,
            std::abs(cx.z) * half.x + std::abs(cy.z) * half.y + std::abs(cz.z) * half.z,
        };

        return AABB{center - world_half, center + world_half};
    }

    bool Frustum::intersects_aabb_world(const AABB& local_bounds, const Matrix4& model) const noexcept {
        if (!local_bounds.is_valid())
            return true;
        return intersects_aabb(transform_aabb(local_bounds, model));
    }
} // namespace star::rendering
