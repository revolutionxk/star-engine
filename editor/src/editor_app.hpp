#pragma once
#include "star/application/application.hpp"

namespace star::editor {
    class EditorApp : public application::Application {
      public:
        explicit EditorApp(const application::ApplicationConfig& config);
        ~EditorApp() override = default;

      protected:
        bool on_initialize() override;
        void on_shutdown() override;
        void on_update(f32 delta_time) override;
    };
} // namespace star::editor
