#pragma once

#include <filesystem>

#include "panel.hpp"

namespace star::editor {
    class EditorWindow;
    
    class ContentBrowserPanel final : public Panel {
      public:
        explicit ContentBrowserPanel(EditorWindow* editor_window);

        void on_imgui_render() override;

      private:
        EditorWindow* m_editor_window;
        std::filesystem::path m_root;
        std::filesystem::path m_current;
    };
} // namespace star::editor
