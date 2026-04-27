#pragma once

#include <imgui.h>

namespace star::editor {
    class EditorTheme {
      public:
        static void apply_theme();
        static bool draw_header(const char* label);
    };

} // namespace star::editor
