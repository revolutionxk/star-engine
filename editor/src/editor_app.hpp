#pragma once
#include "star/application/application.hpp"

namespace star::editor {
    using namespace star::application;

    class EditorApp : public Application {
      public:
        explicit EditorApp(const CommandLineArgs& args);
        ~EditorApp() override = default;

      protected:
        bool on_configure(ApplicationConfig& config);
        bool on_initialize() override;
        void on_shutdown() override;
        void on_update(f32 delta_time) override;
    };
} // namespace star::editor
