#include "editor_app.hpp"

#include <memory>

#include "editor_layer.hpp"

namespace star::editor {
    EditorApp::EditorApp(const application::ApplicationConfig& config) : Application(config) {}

    bool EditorApp::on_initialize() {
        push_layer(std::make_unique<EditorLayer>());

        return true;
    }

    void EditorApp::on_shutdown() {}

    void EditorApp::on_update(f32 delta_time) {}
} // namespace star::editor
