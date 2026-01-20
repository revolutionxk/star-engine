#pragma once

#include "application_config.hpp"
#include "layer_stack.hpp"
#include "star/resources/resource_manager.hpp"
#include "window_manager.hpp"

namespace star::graphics {
    class Device;
}

namespace star::application {
    class CommandLineArgs;
    using namespace star::platform;

    class STAR_EXPORT Application {
      public:
        explicit Application(const CommandLineArgs& args);
        virtual ~Application();

        i32 run();

        void shutdown();

        void push_layer(std::unique_ptr<Layer> layer);

        void push_overlay(std::unique_ptr<Layer> overlay);

        Window& main_window() const {
            return *m_window_manager.get_main_window();
        }

        WindowManager& window_manager() {
            return m_window_manager;
        }

        const WindowManager& window_manager() const {
            return m_window_manager;
        }

        ApplicationConfig& config() {
            return m_config;
        }

        const ApplicationConfig& config() const {
            return m_config;
        }

        const CommandLineArgs& command_line_args() const {
            return m_config.command_line_args;
        }

        graphics::Device& device() const {
            return *m_device;
        }

        static Application& instance() {
            return *s_instance;
        }

        static bool has_instance() {
            return s_instance != nullptr;
        }

      protected:
        virtual bool on_initialize() {
            return true;
        }

        virtual void on_shutdown() {}

        virtual void on_update(f32 delta_time) {}

        virtual void on_post_update(f32 delta_time) {}

      protected:
        ApplicationConfig m_config;

      private:
        WindowManager m_window_manager;
        LayerStack m_layer_stack;
        std::unique_ptr<graphics::Device> m_device;
        std::unique_ptr<resources::ResourceManager> m_resource_manager;

        bool m_running = false;

        static Application* s_instance;
    };
} // namespace star::application

#if defined(STAR_PLATFORM_WINDOWS) && !defined(STAR_CONSOLE_APPLICATION)
    #define STAR_RUN_APPLICATION(AppClass)                                                                             \
        int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {              \
            int argc = __argc;                                                                                         \
            char** argv = __argv;                                                                                      \
            star::application::CommandLineArgs args(argc, const_cast<const char**>(argv));                             \
            auto app = std::make_unique<AppClass>(args);                                                               \
            return app->run();                                                                                         \
        }
#else
    #define STAR_RUN_APPLICATION(AppClass)                                                                             \
        int main(int argc, char** argv) {                                                                              \
            star::application::CommandLineArgs args(argc, const_cast<const char**>(argv));                             \
            auto app = std::make_unique<AppClass>(args);                                                               \
            return app->run();                                                                                         \
        }
#endif
