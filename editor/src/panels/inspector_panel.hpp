#pragma once

#include "editor_panel.hpp"

namespace star::editor {
    class InspectorPanel : public EditorPanel {
    public:
        explicit InspectorPanel(EditorContext &context);

        void on_imgui_render() override;

    private:
        void draw_components();
        void draw_transform_component();
        void draw_camera_component();
        void draw_light_component();
        void draw_mesh_renderer_component();
        void draw_material_component();
        void draw_add_component_menu();
    };
}
