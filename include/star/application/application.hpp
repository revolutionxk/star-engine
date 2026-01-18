#pragma once
#include "application_config.hpp"
#include "layer_stack.hpp"
#include "star/platform/sdl/sdl_window.hpp"

namespace star::application {
    using namespace star::platform;

    class STAR_EXPORT Application {
      public:
        explicit Application(ApplicationConfig config);
        virtual ~Application();

        i32 run();
        void shutdown();

        void push_layer(std::unique_ptr<Layer> layer);
        void push_overlay(std::unique_ptr<Layer> overlay);

        Window& window() const {
            return *m_window;
        }

        static Application& instance() {
            return *s_instance;
        }

      protected:
        virtual bool on_initialize() {
            return true;
        }

        virtual void on_shutdown() {}

        virtual void on_update(f32 delta_time) {}

      private:
        ApplicationConfig m_config;
        std::unique_ptr<Window> m_window;

        LayerStack m_layer_stack;
        bool m_running = false;

        static Application* s_instance;
    };
} // namespace star::application

#if defined(STAR_PLATFORM_WINDOWS) && !defined(STAR_CONSOLE_APPLICATION)
    #define STAR_RUN_APPLICATION(AppClass)                                                                             \
        int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {              \
            star::application::ApplicationConfig config;                                                               \
            auto app = std::make_unique<AppClass>(config);                                                             \
            return app->run();                                                                                         \
        }
#else
    #define STAR_RUN_APPLICATION(AppClass)                                                                             \
        int main(int argc, char** argv) {                                                                              \
            star::application::ApplicationConfig config;                                                               \
            auto app = std::make_unique<AppClass>(config);                                                             \
            return app->run();                                                                                         \
        }
#endif
