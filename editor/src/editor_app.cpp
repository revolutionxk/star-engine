#include "editor_app.hpp"

#include <memory>

#include "editor_window.hpp"

namespace star::editor {
    EditorApp::EditorApp(const CommandLineArgs& args) : Application(args) {}

    bool EditorApp::on_initialize() {
        if (const auto* editor_window = create_window<EditorWindow>(); !editor_window) {
            STAR_LOG_ERROR(LogCategory::Application, "Failed to create editor window");
            return false;
        }

        STAR_LOG_INFO(LogCategory::Application, "Editor initialized successfully");
        return true;
    }

    void EditorApp::on_shutdown() {
        STAR_LOG_INFO(LogCategory::Application, "Editor shutting down");
    }

    void EditorApp::on_update(f32 delta_time) {}
} // namespace star::editor
