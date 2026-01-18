#include "editor_layer.hpp"

namespace star::editor {
    EditorLayer::EditorLayer() : Layer("EditorLayer") {}

    bool EditorLayer::initialize() {
        return true;
    }

    void EditorLayer::shutdown() {}

    void EditorLayer::on_attach() {}

    void EditorLayer::on_detach() {}

    void EditorLayer::update(f32 delta_time) {}

    void EditorLayer::render() {}

    void EditorLayer::on_imgui_render() {}
} // namespace star::editor
