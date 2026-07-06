#include "entity_gizmos.hpp"

#include <algorithm>
#include <cmath>

#include "star/ecs/components/camera.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/math/math.hpp"
#include "star/rendering/components/light.hpp"
#include "star/rendering/debug_renderer.hpp"
#include "star/scene/scene.hpp"

namespace star::editor {
    constexpr f32 DEG2RAD = 0.01745329252f;

    struct WorldTR {
        Vector3 position;
        Quaternion rotation;
    };

    WorldTR world_transform(const flecs::entity entity) {
        const auto* xf = entity.try_get<components::Transform>();
        WorldTR tr{xf ? xf->position : Vector3::zero(), xf ? xf->rotation : Quaternion::identity()};

        if (const flecs::entity parent = entity.parent(); parent.is_valid() && parent.has<components::Transform>()) {
            const WorldTR p = world_transform(parent);
            tr.position = p.position + p.rotation * tr.position;
            tr.rotation = p.rotation * tr.rotation;
        }
        return tr;
    }

    void perpendicular_basis(const Vector3& n, Vector3& a, Vector3& b) {
        const Vector3 seed = std::abs(n.y) < 0.99f ? Vector3::up() : Vector3::right();
        a = n.cross(seed).normalized();
        b = n.cross(a).normalized();
    }

    Vector3 EntityGizmos::world_position(const flecs::entity entity) {
        return world_transform(entity).position;
    }

    void EntityGizmos::draw(scene::Scene& scene, rendering::DebugRenderer& debug,
                            const std::optional<flecs::entity>& selected) {
        const flecs::world& world = scene.world().native();
        const u64 sel = selected && selected->is_valid() ? selected->id() : 0;

        world.query<const components::Camera, const components::Transform>().each(
            [&](const flecs::entity e, const components::Camera&, const components::Transform&) {
                draw_camera(e, debug, e.id() == sel);
            });

        world.query<const components::Light, const components::Transform>().each(
            [&](const flecs::entity e, const components::Light&, const components::Transform&) {
                draw_light(e, debug, e.id() == sel);
            });
    }

    void EntityGizmos::draw_camera(const flecs::entity entity, rendering::DebugRenderer& debug, const bool selected) {
        const auto* cam = entity.try_get<components::Camera>();
        if (!cam)
            return;

        const auto [position, rotation] = world_transform(entity);
        const Vector3 fwd = (rotation * Vector3::forward()).normalized();
        const Vector3 right = (rotation * Vector3::right()).normalized();
        const Vector3 up = (rotation * Vector3::up()).normalized();

        const Color4 color = selected ? Color4{1.0f, 0.85f, 0.2f, 1.0f} : Color4{0.35f, 0.8f, 1.0f, 1.0f};

        const f32 tan_half = std::tan(cam->fov_y * DEG2RAD * 0.5f);
        const f32 n = cam->near_plane;
        const f32 f = std::min(cam->far_plane, n + 6.0f);
        const f32 hn = tan_half * n, wn = hn * cam->aspect_ratio;
        const f32 hf = tan_half * f, wf = hf * cam->aspect_ratio;

        const Vector3 cn = position + fwd * n;
        const Vector3 cf = position + fwd * f;

        const Vector3 near_c[4] = {cn + up * hn - right * wn, cn + up * hn + right * wn, cn - up * hn + right * wn,
                                   cn - up * hn - right * wn};
        const Vector3 far_c[4] = {cf + up * hf - right * wf, cf + up * hf + right * wf, cf - up * hf + right * wf,
                                  cf - up * hf - right * wf};

        for (int i = 0; i < 4; ++i) {
            debug.draw_line(near_c[i], near_c[(i + 1) % 4], color);
            debug.draw_line(far_c[i], far_c[(i + 1) % 4], color);
            debug.draw_line(near_c[i], far_c[i], color);
            debug.draw_line(position, near_c[i], color);
        }

        debug.draw_box(position - fwd * 0.16f, Vector3{0.17f, 0.12f, 0.15f}, rotation, color);
        const Vector3 fin = cn + up * (hn * 1.5f);
        debug.draw_line(near_c[0], fin, color);
        debug.draw_line(near_c[1], fin, color);
    }

    void EntityGizmos::draw_light(const flecs::entity entity, rendering::DebugRenderer& debug, const bool selected) {
        const auto* light = entity.try_get<components::Light>();
        if (!light)
            return;

        const auto [position, rotation] = world_transform(entity);
        Vector3 dir = light->direction;
        const f32 len_sq = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
        dir = len_sq > 1e-6f ? dir.normalized() : Vector3{0.0f, -1.0f, 0.0f};

        const Color4 color = selected ? Color4{1.0f, 0.9f, 0.35f, 1.0f}
                                      : Color4{std::max(light->color.r, 0.45f), std::max(light->color.g, 0.4f),
                                               std::max(light->color.b, 0.2f), 1.0f};

        switch (light->type) {
            case components::Light::Type::Directional: {
                Vector3 a, b;
                perpendicular_basis(dir, a, b);
                constexpr f32 length = 1.7f;
                debug.draw_ray(position, dir, length, color);
                for (int k = 0; k < 4; ++k) {
                    constexpr f32 radius = 0.3f;
                    const f32 ang = static_cast<f32>(k) * 1.57079633f;
                    const Vector3 o = position + a * (std::cos(ang) * radius) + b * (std::sin(ang) * radius);
                    debug.draw_ray(o, dir, length, color);
                }
                break;
            }
            case components::Light::Type::Point: {
                debug.draw_sphere(position, light->range, color, 24);
                break;
            }
            case components::Light::Type::Spot: {
                const f32 half = light->outer_cone_angle_deg * DEG2RAD;
                const f32 length = light->range;
                const f32 base_r = std::tan(half) * length;
                const Vector3 base = position + dir * length;
                Vector3 a, b;
                perpendicular_basis(dir, a, b);

                constexpr int SEG = 24;
                Vector3 prev{};
                for (int k = 0; k <= SEG; ++k) {
                    const f32 ang = static_cast<f32>(k) / SEG * 6.28318531f;
                    const Vector3 p = base + a * (std::cos(ang) * base_r) + b * (std::sin(ang) * base_r);
                    if (k > 0)
                        debug.draw_line(prev, p, color);
                    if (k % 6 == 0)
                        debug.draw_line(position, p, color);
                    prev = p;
                }
                break;
            }
        }
    }
} // namespace star::editor
