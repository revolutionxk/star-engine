#pragma once

#include "star/application/app_window.hpp"

namespace star::editor {
    class EditorWindow : public application::AppWindow {
      public:
        explicit EditorWindow();
        ~EditorWindow() override = default;

      protected:
        bool on_initialize() override;
        void on_shutdown() override;
        void on_update(f32 delta_time) override;
    };
} // namespace star::editor
