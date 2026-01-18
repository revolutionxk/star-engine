#pragma once
#include "star/application/layer.hpp"

namespace star::editor {
    class EditorLayer : public application::Layer {
      public:
        EditorLayer();
        ~EditorLayer() override = default;

        bool initialize() override;
        void shutdown() override;

        void update(f32 delta_time) override;
        void render() override;
        void on_imgui_render() override;

        void on_attach() override;
        void on_detach() override;

      private:
        bool m_viewport_focused = false;
        bool m_viewport_hovered = false;
    };
} // namespace star::editor
