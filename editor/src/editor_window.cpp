#include "editor_window.hpp"

#include "layers/editor_ui_layer.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/scene/camera3d.hpp"
#include "star/scene/components/camera.hpp"
#include "star/scene/mesh_instance3d.hpp"
#include "star/scene/scene_manager.hpp"

namespace star::editor {
    EditorWindow::EditorWindow() : AppWindow("Star Engine Editor", {}) {}

    bool EditorWindow::on_initialize() {
        STAR_LOG_INFO(LogCategory::Editor, "Initializing editor window");

        auto* scene = scene_manager().create_scene("MainScene");
        scene_manager().set_active_scene(scene->name());

        const auto& resource_manager = resources();

        const auto camera = scene->create_node<scene::Camera3D>("EditorCamera");
        camera->set_position(Vector3{0.0f, 2.0f, 5.0f});

        const auto cube_mesh = resource_manager.cube_mesh();

        const auto cube = scene->create_node<scene::MeshInstance3D>("Cube");
        cube->set_mesh(cube_mesh);
        cube->set_position({0.0f, 0.0f, 0.0f});

        const auto plane_mesh = resource_manager.plane_mesh();
        const auto plane = scene->create_node<scene::MeshInstance3D>("Plane");
        plane->set_mesh(plane_mesh);
        plane->set_position({0.0f, -1.0f, 0.0f});

        const auto sphere_mesh = resource_manager.sphere_mesh();
        const auto sphere = scene->create_node<scene::MeshInstance3D>("Sphere");
        sphere->set_mesh(sphere_mesh);
        sphere->set_position({2.0f, 0.0f, 0.0f});

        push_overlay(std::make_unique<EditorUILayer>(this));

        STAR_LOG_INFO(LogCategory::Editor, "Editor window initialized");
        return true;
    }

    void EditorWindow::on_shutdown() {
        STAR_LOG_INFO(LogCategory::Editor, "Editor window shutting down");
    }

    void EditorWindow::on_update(const f32 delta_time) {}
} // namespace star::editor
