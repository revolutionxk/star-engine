#include "scene_picker.hpp"

#include <limits>

#include "star/ecs/components/transform.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/rendering/viewport.hpp"
#include "star/resources/mesh/mesh.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/scene/scene.hpp"

namespace star::editor {
    void ScenePicker::ray_from_viewport(const rendering::Viewport& viewport, const f32 u, const f32 v,
                                        Vector3& out_origin, Vector3& out_direction) {
        const f32 ndc_x = 2.0f * u - 1.0f;
        const f32 ndc_y = 1.0f - 2.0f * v;

        const Matrix4 inv_view_proj = Matrix4::inverse(viewport.projection_matrix() * viewport.view_matrix());

        const Vector4 far_clip{ndc_x, ndc_y, 1.0f, 1.0f};
        const Vector4 far_world = inv_view_proj * far_clip;
        const f32 inv_w = far_world.w != 0.0f ? 1.0f / far_world.w : 1.0f;

        out_origin = viewport.camera_position();
        out_direction =
            (Vector3{far_world.x * inv_w, far_world.y * inv_w, far_world.z * inv_w} - out_origin).normalized();
    }

    Matrix4 ScenePicker::world_matrix(const flecs::entity entity) {
        const auto* local_xf = entity.try_get<components::Transform>();
        const Matrix4 local = local_xf ? local_xf->to_matrix() : Matrix4::identity();

        if (const flecs::entity parent = entity.parent(); parent.is_valid() && parent.has<components::Transform>())
            return world_matrix(parent) * local;
        return local;
    }

    std::optional<flecs::entity> ScenePicker::pick(const Vector3& origin, const Vector3& direction, scene::Scene& scene,
                                                   resources::ResourceManager& resources) {
        std::optional<flecs::entity> best;
        f32 best_t = std::numeric_limits<f32>::max();

        scene.world().native().each(
            [&](const flecs::entity entity, const components::Transform&, const components::MeshRenderer& mr) {
                if (!mr.visible || !mr.mesh.is_valid())
                    return;

                const auto* mesh = resources.get_mesh(mr.mesh);
                if (!mesh || !mesh->bounds.is_valid())
                    return;

                const Matrix4 inv_world = Matrix4::inverse(world_matrix(entity));
                const Vector4 local_origin = inv_world * Vector4{origin, 1.0f};
                const Vector4 local_dir = inv_world * Vector4{direction, 0.0f};

                const auto t = mesh->bounds.intersect_ray(Vector3{local_origin.x, local_origin.y, local_origin.z},
                                                          Vector3{local_dir.x, local_dir.y, local_dir.z});
                if (t && *t < best_t) {
                    best_t = *t;
                    best = entity;
                }
            });

        return best;
    }
} // namespace star::editor
