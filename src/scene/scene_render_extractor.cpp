#include "star/scene/scene_render_extractor.hpp"

#include <unordered_map>

#include "star/core/common.hpp"
#include "star/ecs/components/camera.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/ecs/components/transform_interpolation.hpp"
#include "star/rendering/components/atmosphere.hpp"
#include "star/rendering/components/light.hpp"
#include "star/rendering/components/material_instance.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/rendering/render_scene.hpp"
#include "star/scene/scene.hpp"

namespace star::scene {
    namespace {
        rendering::LightSnapshot::Type to_snapshot(const components::Light::Type t) noexcept {
            switch (t) {
                case components::Light::Type::Point:
                    return rendering::LightSnapshot::Type::Point;
                case components::Light::Type::Spot:
                    return rendering::LightSnapshot::Type::Spot;
                case components::Light::Type::Directional:
                default:
                    return rendering::LightSnapshot::Type::Directional;
            }
        }
    } // namespace

    struct SceneRenderExtractor::QueryCache {
        flecs::world* world{nullptr};
        flecs::query<const components::Camera, const components::Transform, const components::PrimaryCamera> cameras;
        flecs::query<const components::Light, const components::Transform> lights;
        flecs::query<const components::Atmosphere> atmospheres;
        flecs::query<const components::MeshRenderer, const components::Transform> renderables;

        void rebuild_for(flecs::world& w) {
            world = &w;
            cameras = w.query<const components::Camera, const components::Transform, const components::PrimaryCamera>();
            lights = w.query<const components::Light, const components::Transform>();
            atmospheres = w.query<const components::Atmosphere>();
            renderables = w.query<const components::MeshRenderer, const components::Transform>();
        }

        static Matrix4 local_matrix(const flecs::entity e, const f32 alpha) {
            const auto* xf = e.try_get<components::Transform>();
            if (!xf)
                return Matrix4::identity();

            const auto* interp = alpha < 1.0f ? e.try_get<components::TransformInterpolation>() : nullptr;
            if (!interp)
                return xf->to_matrix();

            const Vector3 position = interp->previous_position.lerp(xf->position, alpha);
            const Quaternion rotation = interp->previous_rotation.slerp(xf->rotation, alpha);
            const Vector3 scale = interp->previous_scale.lerp(xf->scale, alpha);
            return Matrix4::translate(position) * Matrix4::from_quaternion(rotation) * Matrix4::scale(scale);
        }

        static Matrix4 world_matrix(const flecs::entity e, const f32 alpha, std::unordered_map<u64, Matrix4>& cache) {
            if (const auto it = cache.find(e.id()); it != cache.end())
                return it->second;

            const Matrix4 local = local_matrix(e, alpha);

            Matrix4 world = local;
            if (const flecs::entity parent = e.parent(); parent.is_valid() && parent.has<components::Transform>())
                world = world_matrix(parent, alpha, cache) * local;

            cache.emplace(e.id(), world);
            return world;
        }
    };

    SceneRenderExtractor::SceneRenderExtractor() : m_cache(std::make_unique<QueryCache>()) {}

    SceneRenderExtractor::~SceneRenderExtractor() = default;

    void SceneRenderExtractor::reset() {
        m_cache = std::make_unique<QueryCache>();
    }

    SceneRenderExtractor::SceneRenderExtractor(SceneRenderExtractor&&) noexcept = default;
    SceneRenderExtractor& SceneRenderExtractor::operator=(SceneRenderExtractor&&) noexcept = default;

    void SceneRenderExtractor::extract(Scene& scene, rendering::RenderScene& out, const f32 alpha) const {
        out.clear();
        out.name = scene.name();
        out.active = scene.is_active();
        if (!out.active) {
            return;
        }

        auto& world = scene.world().native();
        if (m_cache->world != &world) {
            m_cache->rebuild_for(world);
        }

        m_cache->cameras.each([&](flecs::entity, const components::Camera& cam, const components::Transform& xf,
                                  const components::PrimaryCamera&) {
            if (out.primary_camera.has_value()) {
                return;
            }
            rendering::CameraSnapshot snap;
            snap.fov_y = cam.fov_y;
            snap.near_plane = cam.near_plane;
            snap.far_plane = cam.far_plane;
            snap.aspect_ratio = cam.aspect_ratio;
            snap.position = xf.position;
            snap.rotation = xf.rotation;
            snap.scale = xf.scale;
            out.primary_camera = snap;
        });

        m_cache->lights.each([&](flecs::entity, const components::Light& light, const components::Transform& xf) {
            rendering::LightSnapshot snap;
            snap.type = to_snapshot(light.type);
            snap.position = xf.position;
            const f32 dir_len_sq = light.direction.x * light.direction.x + light.direction.y * light.direction.y +
                                   light.direction.z * light.direction.z;
            snap.direction = dir_len_sq > 1e-6f ? light.direction.normalized() : Vector3{0.0f, -1.0f, 0.0f};
            snap.color = light.color;
            snap.intensity = light.intensity;
            snap.ambient_intensity = light.ambient_intensity;
            snap.range = light.range;
            snap.inner_cone_angle_deg = light.inner_cone_angle_deg;
            snap.outer_cone_angle_deg = light.outer_cone_angle_deg;
            snap.cast_shadows = light.cast_shadows;
            out.lights.push_back(snap);
        });

        m_cache->atmospheres.each([&](flecs::entity, const components::Atmosphere& atm) {
            if (out.atmosphere.has_value()) {
                return;
            }
            rendering::AtmosphereSnapshot snap;
            snap.enabled = atm.enabled;
            snap.turbidity = atm.turbidity;
            snap.sun_elevation = atm.sun_elevation;
            snap.sun_azimuth = atm.sun_azimuth;
            snap.sun_intensity = atm.sun_intensity;
            out.atmosphere = snap;
        });

        out.renderables.reserve(64);
        std::unordered_map<u64, Matrix4> world_cache;
        m_cache->renderables.each(
            [&](const flecs::entity e, const components::MeshRenderer& mr, const components::Transform& xf) {
                if (!mr.visible || !mr.mesh.is_valid()) {
                    return;
                }
                const Matrix4 world = QueryCache::world_matrix(e, alpha, world_cache);

                rendering::RenderableSnapshot snap;
                snap.model_matrix = world;
                snap.world_position = Vector3{world[3].x, world[3].y, world[3].z};
                snap.entity_id = e.id();
                snap.mesh = mr.mesh;
                snap.material = mr.material;
                snap.layer = mr.layer;
                if (const auto* mi = e.try_get<components::MaterialInstance>()) {
                    snap.parameters = mi->parameters;
                    if (mi->material.is_valid()) {
                        snap.material = mi->material;
                    }
                }
                out.renderables.push_back(std::move(snap));
            });
    }
} // namespace star::scene
