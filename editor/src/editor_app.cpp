#include "editor_app.hpp"

#include <memory>

#include "editor_layer.hpp"

namespace star::editor {
    EditorApp::EditorApp(const CommandLineArgs& args) : Application(args) {}

    bool EditorApp::on_initialize() {
        push_layer(std::make_unique<EditorLayer>());

        STAR_LOG_INFO(LogCategory::Application, "Editor initialized successfully");

        return true;
    }

    void EditorApp::on_shutdown() {
        STAR_LOG_INFO(LogCategory::Application, "Editor shutting down");
    }

    void EditorApp::on_update(f32 delta_time) {}
} // namespace star::editor
