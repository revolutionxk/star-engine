#include "editor_window.hpp"

#include "layers/editor_ui_layer.hpp"
#include "star/rendering/components/atmosphere.hpp"
#include "star/rendering/components/light.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/scene/components/camera.hpp"
#include "star/scene/scene_manager.hpp"

namespace star::editor {
    EditorWindow::EditorWindow() : AppWindow("Star Engine Editor", {}) {}

    bool EditorWindow::on_initialize() {
        STAR_LOG_INFO(LogCategory::Editor, "Initializing editor window");

        const auto* scene = scene_manager().create_scene("MainScene");
        scene_manager().set_active_scene(scene->name());

        const auto& resource_manager = resources();

        scene->create_entity("EditorCamera")
            .set<components::Transform>({
                .position = Vector3{0.0f, 2.0f, 5.0f},
            })
            .set<components::Camera>({})
            .add<components::PrimaryCamera>();

        scene->create_entity("DirectionalLight")
            .set<components::Transform>({
                .position = Vector3{0.0f, 4.0f, 0.0f},
                .rotation = Quaternion{0.0f, 0.0f, 0.0f, 1.0f},
            })
            .set<components::Light>({
                .type = components::Light::Type::Directional,
                .direction = Vector3{0.0f, -1.0f, 0.0f},
                .color = Vector3{1.0f, 1.0f, 1.0f},
                .intensity = 1.0f,
                .ambient_intensity = 0.03f,
                .cast_shadows = true,
            });

        const auto cube_mesh = resource_manager.cube_mesh();
        scene->create_entity("Cube")
            .set<components::Transform>({
                .position = Vector3{0.0f, 0.0f, 0.0f},
            })
            .set<components::MeshRenderer>({
                .mesh = cube_mesh,
                .material = resource_manager.default_material(),
            });

        const auto plane_mesh = resource_manager.plane_mesh();
        scene->create_entity("Plane")
            .set<components::Transform>({
                .position = Vector3{0.0f, -1.0f, 0.0f},
                .scale = Vector3{10.0f, 1.0f, 10.0f},
            })
            .set<components::MeshRenderer>({
                .mesh = plane_mesh,
                .material = resource_manager.default_material(),
            });

        const auto sphere_mesh = resource_manager.sphere_mesh();
        scene->create_entity("Sphere")
            .set<components::Transform>({
                .position = Vector3{2.0f, 0.0f, 0.0f},
            })
            .set<components::MeshRenderer>({
                .mesh = sphere_mesh,
                .material = resource_manager.default_material(),
            });

        scene->create_entity("Sky").set<components::Atmosphere>({});

        push_overlay(std::make_unique<EditorUILayer>(this));

        STAR_LOG_INFO(LogCategory::Editor, "Editor window initialized");
        return true;
    }

    void EditorWindow::on_shutdown() {
        STAR_LOG_INFO(LogCategory::Editor, "Editor window shutting down");
    }

    void EditorWindow::on_update(const f32 delta_time) {}
} // namespace star::editor
