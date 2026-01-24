#pragma once

#include "app_window.hpp"
#include "application_config.hpp"
#include "application_context.hpp"
#include "game_loop.hpp"
#include "layer_stack.hpp"
#include "star/rendering/renderer.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/systems/render_system.hpp"

namespace star::scene {
    class SceneManager;
}

namespace star::graphics {
    class Device;
}

namespace star::platform {
    class Window;
} // namespace star::platform

namespace star::application {
    class CommandLineArgs;
    class WindowManager;

    class STAR_EXPORT Application {
      public:
        explicit Application(const CommandLineArgs& args);
        virtual ~Application();

        i32 run();
        void shutdown();

        template<typename T, typename... Args>
        T* create_window(Args&&... args) {
            static_assert(std::is_base_of_v<AppWindow, T>, "T must derive from AppWindow");
            auto window = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = window.get();

            if (!window->initialize()) {
                STAR_LOG_ERROR(LogCategory::Application, "Failed to initialize window");
                return nullptr;
            }

            m_app_windows.push_back(std::move(window));
            STAR_LOG_INFO(LogCategory::Application, "Window created: {}", ptr->title());
            return ptr;
        }

        [[nodiscard]] platform::Window& main_window() const;
        [[nodiscard]] WindowManager& window_manager() const;

        [[nodiscard]] ApplicationConfig& config() {
            return m_config;
        }

        [[nodiscard]] const ApplicationConfig& config() const {
            return m_config;
        }

        [[nodiscard]] const CommandLineArgs& command_line_args() const {
            return m_config.command_line_args;
        }

        [[nodiscard]] ApplicationContext& context() {
            return m_context;
        }

        [[nodiscard]] const ApplicationContext& context() const {
            return m_context;
        }

        [[nodiscard]] GameLoop& game_loop() {
            return *m_game_loop;
        }

        [[nodiscard]] const GameLoop& game_loop() const {
            return *m_game_loop;
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

      private:
        bool initialize_subsystems();
        void shutdown_subsystems();
        void update_frame(f32 delta_time);
        void render_frame() const;

      protected:
        ApplicationConfig m_config;

      private:
        ApplicationContext m_context;

        std::unique_ptr<WindowManager> m_window_manager;
        std::vector<std::unique_ptr<AppWindow>> m_app_windows;
        std::unique_ptr<GameLoop> m_game_loop;

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
