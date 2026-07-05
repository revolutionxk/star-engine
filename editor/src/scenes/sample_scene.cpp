#include "sample_scene.hpp"

#include <memory>
#include <string>

#include "star/ecs/components/camera.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/physics/components/collider.hpp"
#include "star/physics/components/rigid_body.hpp"
#include "star/project/project.hpp"
#include "star/rendering/components/atmosphere.hpp"
#include "star/rendering/components/light.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/resources/material/material.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/scene/scene.hpp"

namespace star::editor {
    void build_empty_scene(const scene::Scene& scene) {
        scene.create_entity("EditorCamera")
            .set<components::Transform>({.position = Vector3{6.0f, 4.0f, 9.0f}})
            .set<components::Camera>({})
            .add<components::PrimaryCamera>();

        scene.create_entity("Sky").set<components::Atmosphere>({
            .turbidity = 2.5f,
            .sun_elevation = 0.6f,
            .sun_azimuth = 0.4f,
            .sun_intensity = 1.6f,
        });

        scene.create_entity("Sun")
            .set<components::Transform>({.position = Vector3{0.0f, 8.0f, 0.0f}})
            .set<components::Light>({
                .type = components::Light::Type::Directional,
                .direction = Vector3{-0.4f, -0.8f, -0.45f}.normalized(),
                .color = Color4{1.0f, 0.97f, 0.92f, 1.0f},
                .intensity = 0.9f,
                .ambient_intensity = 0.0f,
                .cast_shadows = true,
            });
    }

    void build_sample_scene(const scene::Scene& scene, resources::ResourceManager& resources) {
        const auto cube_mesh = resources.cube_mesh();
        const auto plane_mesh = resources.plane_mesh();
        const auto sphere_mesh = resources.sphere_mesh();

        const auto material = [&](const std::string& name, const Vector4& albedo, const f32 metallic,
                                  const f32 roughness, const Vector4& emissive = {0.0f, 0.0f, 0.0f, 0.0f}) {
            auto asset = std::make_unique<resources::Material>();
            asset->shader = resources.default_shader();
            asset->albedo_color = albedo;
            asset->emissive_color = emissive;
            asset->metallic = metallic;
            asset->roughness = roughness;

            const std::string path =
                std::string{project::layout::MATERIALS} + "/" + name + std::string{project::layout::MATERIAL_EXTENSION};
            const auto handle = resources.create_material(path, std::move(asset));
            (void)resources.save_material(handle, path);
            return handle;
        };

        build_empty_scene(scene);

        const auto ground_mat = material("Ground", Vector4{0.35f, 0.35f, 0.38f, 1.0f}, 0.0f, 0.85f);
        scene.create_entity("Ground")
            .set<components::Transform>({
                .position = Vector3{0.0f, -1.0f, 0.0f},
                .scale = Vector3{40.0f, 1.0f, 40.0f},
            })
            .set<components::MeshRenderer>({.mesh = plane_mesh, .material = ground_mat});

        for (int i = 0; i < 5; ++i) {
            const f32 roughness = 0.05f + static_cast<f32>(i) * 0.22f;
            const auto mat =
                material("MetalGold_" + std::to_string(i), Vector4{0.95f, 0.78f, 0.45f, 1.0f}, 1.0f, roughness);
            scene.create_entity("MetalSphere_" + std::to_string(i))
                .set<components::Transform>({
                    .position = Vector3{-4.0f + static_cast<f32>(i) * 2.0f, 0.0f, -3.0f},
                    .scale = Vector3{0.8f, 0.8f, 0.8f},
                })
                .set<components::MeshRenderer>({.mesh = sphere_mesh, .material = mat});
        }

        for (int i = 0; i < 5; ++i) {
            const f32 roughness = 0.05f + static_cast<f32>(i) * 0.22f;
            const auto mat =
                material("PlasticBlue_" + std::to_string(i), Vector4{0.15f, 0.45f, 0.85f, 1.0f}, 0.0f, roughness);
            scene.create_entity("PlasticSphere_" + std::to_string(i))
                .set<components::Transform>({
                    .position = Vector3{-4.0f + static_cast<f32>(i) * 2.0f, 0.0f, -1.0f},
                    .scale = Vector3{0.8f, 0.8f, 0.8f},
                })
                .set<components::MeshRenderer>({.mesh = sphere_mesh, .material = mat});
        }

        const auto hero_mat = material("HeroRed", Vector4{0.85f, 0.2f, 0.18f, 1.0f}, 0.0f, 0.35f);
        scene.create_entity("HeroCube")
            .set<components::Transform>({
                .position = Vector3{0.0f, 0.0f, 1.5f},
                .scale = Vector3{0.8f, 0.8f, 0.8f},
            })
            .set<components::MeshRenderer>({.mesh = cube_mesh, .material = hero_mat});

        scene.create_entity("PhysicsGround")
            .set<components::Transform>({.position = Vector3{0.0f, -1.5f, 0.0f}})
            .set<components::RigidBody>({.motion = physics::MotionType::Static})
            .set<components::Collider>(
                {.shape = physics::ColliderShape::Box, .half_extents = Vector3{20.0f, 0.5f, 20.0f}});

        const auto physics_mat = material("GreenPhysics", Vector4{0.25f, 0.85f, 0.35f, 1.0f}, 0.0f, 0.4f);
        scene.create_entity("PhysicsCube")
            .set<components::Transform>({.position = Vector3{0.0f, 6.0f, 3.0f}, .scale = Vector3{0.8f, 0.8f, 0.8f}})
            .set<components::MeshRenderer>({.mesh = cube_mesh, .material = physics_mat})
            .set<components::RigidBody>({.motion = physics::MotionType::Dynamic})
            .set<components::Collider>(
                {.shape = physics::ColliderShape::Box, .half_extents = Vector3{0.4f, 0.4f, 0.4f}});
    }
} // namespace star::editor
