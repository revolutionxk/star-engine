#include "editor_window.hpp"

#include "layers/editor_ui_layer.hpp"
#include "star/ecs/components/camera.hpp"
#include "star/rendering/components/atmosphere.hpp"
#include "star/rendering/components/light.hpp"
#include "star/rendering/components/material_instance.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/rendering/material_property.hpp"
#include "star/rendering/shader_uniforms.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/scene/scene_manager.hpp"

namespace star::editor {
    EditorWindow::EditorWindow() : AppWindow("Star Engine Editor", {}) {}

    bool EditorWindow::on_initialize() {
        STAR_LOG_INFO(LogCategory::Editor, "Initializing editor window");

        const auto* scene = scene_manager().create_scene("MainScene");
        scene_manager().set_active_scene(scene->name());

        const auto& resource_manager = resources();
        const auto cube_mesh = resource_manager.cube_mesh();
        const auto plane_mesh = resource_manager.plane_mesh();
        const auto sphere_mesh = resource_manager.sphere_mesh();
        const auto default_mat = resource_manager.default_material();

        const auto pbr_instance = [&](const Vector4& color, const f32 metallic, const f32 roughness,
                                      const Vector4& emissive = {0, 0, 0, 0}) {
            components::MaterialInstance inst;
            inst.material = default_mat;
            inst.parameters.push_back({std::string(rendering::uniforms::BASE_COLOR), color});
            inst.parameters.push_back(
                {std::string(rendering::uniforms::MATERIAL_PARAMS), Vector4{metallic, roughness, 0.0f, 0.0f}});
            inst.parameters.push_back({std::string(rendering::uniforms::EMISSIVE_COLOR), emissive});
            return inst;
        };

        scene->create_entity("EditorCamera")
            .set<components::Transform>({
                .position = Vector3{6.0f, 4.0f, 9.0f},
            })
            .set<components::Camera>({})
            .add<components::PrimaryCamera>();

        scene->create_entity("Sky").set<components::Atmosphere>({
            .turbidity = 2.5f,
            .sun_elevation = 0.6f,
            .sun_azimuth = 0.4f,
            .sun_intensity = 1.6f,
        });

        scene->create_entity("Sun")
            .set<components::Transform>({.position = Vector3{0.0f, 8.0f, 0.0f}})
            .set<components::Light>({
                .type = components::Light::Type::Directional,
                .direction = Vector3{-0.4f, -0.8f, -0.45f}.normalized(),
                .color = Color4{1.0f, 0.97f, 0.92f, 1.0f},
                .intensity = 0.9f,
                .ambient_intensity = 0.0f,
                .cast_shadows = true,
            });

        scene->create_entity("Ground")
            .set<components::Transform>({
                .position = Vector3{0.0f, -1.0f, 0.0f},
                .scale = Vector3{40.0f, 1.0f, 40.0f},
            })
            .set<components::MeshRenderer>({.mesh = plane_mesh, .material = default_mat})
            .set<components::MaterialInstance>(pbr_instance(Vector4{0.35f, 0.35f, 0.38f, 1.0f}, 0.0f, 0.85f));

        for (int i = 0; i < 5; ++i) {
            const f32 roughness = 0.05f + static_cast<f32>(i) * 0.22f;
            scene->create_entity("MetalSphere_" + std::to_string(i))
                .set<components::Transform>({
                    .position = Vector3{-4.0f + static_cast<f32>(i) * 2.0f, 0.0f, -3.0f},
                    .scale = Vector3{0.8f, 0.8f, 0.8f},
                })
                .set<components::MeshRenderer>({.mesh = sphere_mesh, .material = default_mat})
                .set<components::MaterialInstance>(pbr_instance(Vector4{0.95f, 0.78f, 0.45f, 1.0f}, 1.0f, roughness));
        }

        for (int i = 0; i < 5; ++i) {
            const f32 roughness = 0.05f + static_cast<f32>(i) * 0.22f;
            scene->create_entity("PlasticSphere_" + std::to_string(i))
                .set<components::Transform>({
                    .position = Vector3{-4.0f + static_cast<f32>(i) * 2.0f, 0.0f, -1.0f},
                    .scale = Vector3{0.8f, 0.8f, 0.8f},
                })
                .set<components::MeshRenderer>({.mesh = sphere_mesh, .material = default_mat})
                .set<components::MaterialInstance>(pbr_instance(Vector4{0.15f, 0.45f, 0.85f, 1.0f}, 0.0f, roughness));
        }

        scene->create_entity("HeroCube")
            .set<components::Transform>({
                .position = Vector3{0.0f, 0.0f, 1.5f},
                .scale = Vector3{0.8f, 0.8f, 0.8f},
            })
            .set<components::MeshRenderer>({.mesh = cube_mesh, .material = default_mat})
            .set<components::MaterialInstance>(pbr_instance(Vector4{0.85f, 0.2f, 0.18f, 1.0f}, 0.0f, 0.35f));

        constexpr Vector3 room_center{6.0f, 0.0f, 2.0f};
        constexpr f32 room_size = 2.0f;
        constexpr f32 wall_thickness = 0.1f;
        constexpr Vector4 wall_color{0.08f, 0.08f, 0.10f, 1.0f};
        const auto wall_mat = pbr_instance(wall_color, 0.0f, 0.7f);

        scene->create_entity("Room_Floor")
            .set<components::Transform>({
                .position = room_center + Vector3{0.0f, -room_size, 0.0f},
                .scale = Vector3{room_size * 2.0f, wall_thickness, room_size * 2.0f},
            })
            .set<components::MeshRenderer>({.mesh = cube_mesh, .material = default_mat})
            .set<components::MaterialInstance>(wall_mat);

        scene->create_entity("Room_Ceiling")
            .set<components::Transform>({
                .position = room_center + Vector3{0.0f, room_size, 0.0f},
                .scale = Vector3{room_size * 2.0f, wall_thickness, room_size * 2.0f},
            })
            .set<components::MeshRenderer>({.mesh = cube_mesh, .material = default_mat})
            .set<components::MaterialInstance>(wall_mat);

        scene->create_entity("Room_Back")
            .set<components::Transform>({
                .position = room_center + Vector3{0.0f, 0.0f, -room_size},
                .scale = Vector3{room_size * 2.0f, room_size * 2.0f, wall_thickness},
            })
            .set<components::MeshRenderer>({.mesh = cube_mesh, .material = default_mat})
            .set<components::MaterialInstance>(wall_mat);

        scene->create_entity("Room_Left")
            .set<components::Transform>({
                .position = room_center + Vector3{-room_size, 0.0f, 0.0f},
                .scale = Vector3{wall_thickness, room_size * 2.0f, room_size * 2.0f},
            })
            .set<components::MeshRenderer>({.mesh = cube_mesh, .material = default_mat})
            .set<components::MaterialInstance>(wall_mat);

        scene->create_entity("Room_Right")
            .set<components::Transform>({
                .position = room_center + Vector3{room_size, 0.0f, 0.0f},
                .scale = Vector3{wall_thickness, room_size * 2.0f, room_size * 2.0f},
            })
            .set<components::MeshRenderer>({.mesh = cube_mesh, .material = default_mat})
            .set<components::MaterialInstance>(wall_mat);

        scene->create_entity("Room_LightOrb")
            .set<components::Transform>({
                .position = room_center + Vector3{0.0f, 0.5f, 0.0f},
                .scale = Vector3{0.2f, 0.2f, 0.2f},
            })
            .set<components::MeshRenderer>({.mesh = sphere_mesh, .material = default_mat})
            .set<components::MaterialInstance>(
                pbr_instance(Vector4{1.0f, 0.7f, 0.3f, 1.0f}, 0.0f, 1.0f, Vector4{0.65f, 0.42f, 0.15f, 1.0f}));

        scene->create_entity("Room_PointLight")
            .set<components::Transform>({
                .position = room_center + Vector3{0.0f, 0.5f, 0.0f},
            })
            .set<components::Light>({
                .type = components::Light::Type::Point,
                .color = Color4{1.0f, 0.6f, 0.25f, 1.0f},
                .intensity = 40.0f,
                .ambient_intensity = 0.0f,
                .range = 9.0f,
            });

        scene->create_entity("Room_Cube")
            .set<components::Transform>({
                .position = room_center + Vector3{0.6f, -0.6f, -0.4f},
                .scale = Vector3{0.4f, 0.4f, 0.4f},
            })
            .set<components::MeshRenderer>({.mesh = cube_mesh, .material = default_mat})
            .set<components::MaterialInstance>(pbr_instance(Vector4{0.9f, 0.9f, 0.92f, 1.0f}, 0.0f, 0.4f));

        scene->create_entity("SpotLight")
            .set<components::Transform>({.position = Vector3{-3.0f, 4.0f, 4.0f}})
            .set<components::Light>({
                .type = components::Light::Type::Spot,
                .direction = Vector3{3.0f, -4.0f, -2.5f}.normalized(),
                .color = Color4{0.25f, 0.55f, 1.0f, 1.0f},
                .intensity = 10.0f,
                .ambient_intensity = 0.0f,
                .range = 16.0f,
                .inner_cone_angle_deg = 23.0f,
                .outer_cone_angle_deg = 41.0f,
            });

        push_overlay(std::make_unique<EditorUILayer>(this));

        STAR_LOG_INFO(LogCategory::Editor, "Editor window initialized");
        return true;
    }

    void EditorWindow::on_shutdown() {
        STAR_LOG_INFO(LogCategory::Editor, "Editor window shutting down");
    }

    void EditorWindow::on_update(const f32 delta_time) {}
} // namespace star::editor
