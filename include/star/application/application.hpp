#pragma once

#include "app_window.hpp"
#include "application_config.hpp"
#include "application_context.hpp"
#include "game_loop.hpp"
#include "star/core/job_system.hpp"

namespace star::platform {
    class Input;
}

namespace star::resources {
    class ResourceManager;
}

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
        [[nodiscard]] platform::Input& input_manager() const;

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

        [[nodiscard]] JobSystem& jobs() const {
            return *m_jobs;
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

        virtual void on_fixed_update(f32 fixed_dt) {}

        virtual void on_update(f32 delta_time) {}

        virtual void on_post_update(f32 delta_time) {}

      private:
        bool initialize_subsystems();
        void shutdown_subsystems();
        void update_frame(f32 delta_time);
        void fixed_update_frame(f32 fixed_dt);
        void render_frame(f32 alpha) const;

      protected:
        ApplicationConfig m_config;

      private:
        ApplicationContext m_context;

        std::unique_ptr<WindowManager> m_window_manager;
        std::unique_ptr<platform::Input> m_input_manager;
        std::vector<std::unique_ptr<AppWindow>> m_app_windows;
        std::unique_ptr<GameLoop> m_game_loop;
        std::unique_ptr<JobSystem> m_jobs;

        std::shared_ptr<graphics::Device> m_device;
        std::shared_ptr<resources::ResourceManager> m_resource_manager;

        bool m_running = false;
        bool m_subsystems_shut_down = false;
        static Application* s_instance;
    };
} // namespace star::application

#if defined(STAR_PLATFORM_WINDOWS) && !defined(STAR_CONSOLE_APPLICATION)
    #define STAR_RUN_APPLICATION(AppClass)                                                                             \
        extern "C" int __stdcall WinMain(void*, void*, char*, int) {                                                  \
            const star::application::CommandLineArgs args(__argc, const_cast<const char**>(__argv));                  \
            return std::make_unique<AppClass>(args)->run();                                                           \
        }
#else
    #define STAR_RUN_APPLICATION(AppClass)                                                                             \
        int main(int argc, char** argv) {                                                                             \
            const star::application::CommandLineArgs args(argc, const_cast<const char**>(argv));                      \
            return std::make_unique<AppClass>(args)->run();                                                           \
        }
#endif
