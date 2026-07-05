#pragma once

#include "panel.hpp"

namespace star::rendering {
    class Viewport;
} // namespace star::rendering

namespace star::editor {
    class GamePanel final : public Panel {
      public:
        GamePanel() : Panel("Game") {}

        void set_viewport(rendering::Viewport* viewport) noexcept {
            m_viewport = viewport;
        }

        void on_imgui_render() override;

      private:
        void handle_viewport_resize() const;

        rendering::Viewport* m_viewport = nullptr;
    };
} // namespace star::editor
