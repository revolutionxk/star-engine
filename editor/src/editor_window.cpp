#include "editor_window.hpp"

#include "layers/editor_ui_layer.hpp"
#include "layers/viewport_layer.hpp"
#include "star/scene/camera3d.hpp"
#include "star/scene/components/camera.hpp"
#include "star/scene/scene_manager.hpp"

namespace star::editor {
    EditorWindow::EditorWindow() : AppWindow("Star Engine Editor", {}) {}

    bool EditorWindow::on_initialize() {
        STAR_LOG_INFO(LogCategory::Editor, "Initializing editor window");

        auto* scene = scene_manager().create_scene("MainScene");
        scene_manager().set_active_scene(scene->name());

        const auto camera = scene->create_node<scene::Camera3D>("EditorCamera");
        camera->set_position(Vector3{0.0f, 2.0f, 5.0f});

        push_layer(std::make_unique<ViewportLayer>(this));
        push_overlay(std::make_unique<EditorUILayer>(this));

        STAR_LOG_INFO(LogCategory::Editor, "Editor window initialized");
        return true;
    }

    void EditorWindow::on_shutdown() {
        STAR_LOG_INFO(LogCategory::Editor, "Editor window shutting down");
    }

    void EditorWindow::on_update(const f32 delta_time) {}
} // namespace star::editor
