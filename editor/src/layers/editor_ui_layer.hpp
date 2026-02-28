#pragma once

#include <memory>

#include "../core/panel_manager.hpp"
#include "../panels/component_inspector.hpp"
#include "editor_window.hpp"
#include "star/application/layer.hpp"
#include "star/ecs/component_registry.hpp"
#include "star/rendering/viewport.hpp"

namespace star::editor {
    class EditorUILayer final : public application::Layer {
      public:
        explicit EditorUILayer(EditorWindow* editor_window);
        ~EditorUILayer() override = default;

        EditorUILayer(const EditorUILayer&) = delete;
        EditorUILayer& operator=(const EditorUILayer&) = delete;
        EditorUILayer(EditorUILayer&&) = delete;
        EditorUILayer& operator=(EditorUILayer&&) = delete;

        bool initialize() override;
        void shutdown() override;
        void update(f32 dt) override;
        void on_imgui_render() override;
        void on_imgui_init() override;

      private:
        static void setup_dockspace();
        void render_main_menu_bar() const;
        void initialize_panels();

        EditorWindow* m_editor_window = nullptr;
        std::unique_ptr<rendering::Viewport> m_viewport;
        PanelManager m_panel_manager;
        ecs::ComponentRegistry m_component_registry;
        ComponentInspector m_component_inspector;
    };
} // namespace star::editor
