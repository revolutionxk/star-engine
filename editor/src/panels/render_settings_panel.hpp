#pragma once

#include "panel.hpp"

namespace star::editor {
    class EditorWindow;
    
    class RenderSettingsPanel final : public Panel {
      public:
        explicit RenderSettingsPanel(EditorWindow* editor_window)
            : Panel("Rendering"), m_editor_window(editor_window) {}

        void on_imgui_render() override;

      private:
        EditorWindow* m_editor_window = nullptr;
    };
} // namespace star::editor
