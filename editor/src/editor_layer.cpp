#include "editor_layer.hpp"

#include "star/application/application.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/scene/camera3d.hpp"
#include "star/scene/components/camera.hpp"
#include "star/scene/components/transform.hpp"
#include "star/scene/mesh_instance3d.hpp"
#include "star/scene/scene_manager.hpp"

namespace star::editor {
    EditorLayer::EditorLayer() : Layer("EditorLayer") {}

    bool EditorLayer::initialize() {
        const auto& app = application::Application::instance();
        const auto& resource_manager = app.resource_manager();
        auto& scene_manager = app.scene_manager();

        auto* scene = scene_manager.get_scene("MainScene");
        if (!scene) {
            scene = scene_manager.create_scene("MainScene");
        }

        scene->set_active(true);

        const auto camera = scene->create_node<scene::Camera3D>("MainCamera");
        camera->set_position({0.0f, 2.0f, 5.0f});

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

        return true;
    }

    void EditorLayer::shutdown() {}

    void EditorLayer::on_attach() {}

    void EditorLayer::on_detach() {}

    void EditorLayer::update(f32 delta_time) {}

    void EditorLayer::render() {}

    void EditorLayer::on_imgui_render() {}
} // namespace star::editor
