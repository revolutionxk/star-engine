#include "star/application/application.hpp"

#include "star/application/command_line_args.hpp"
#include "star/application/window_manager.hpp"
#include "star/graphics/command_buffer.hpp"
#include "star/graphics/device.hpp"
#include "star/platform/input/input.hpp"
#include "star/rendering/renderer.hpp"
#include "star/scene/scene_manager.hpp"

namespace star::application {

    Application* Application::s_instance = nullptr;

    Application::Application(const CommandLineArgs& args) {
        Logger::initialize(LogLevel::Debug, LogLevel::Trace, "logs/star_engine.log");

        STAR_CORE_ASSERT(!s_instance, "Application already exists!");
        s_instance = this;

        STAR_LOG_INFO(LogCategory::Application, "Application starting...");
        STAR_LOG_INFO(LogCategory::Application, "Executable: {}", args.get_executable_path());

        m_config.command_line_args = args;

        if (args.size() > 0) {
            STAR_LOG_INFO(LogCategory::Application, "Command line arguments:");
            for (const auto& arg : m_config.command_line_args.get_all()) {
                STAR_LOG_INFO(LogCategory::Application, "  {}", arg);
            }
        }

        if (!initialize_subsystems()) {
            STAR_LOG_ERROR(LogCategory::Application, "Failed to initialize subsystems");
            return;
        }

        m_running = true;
    }

    Application::~Application() {
        STAR_LOG_INFO(LogCategory::Application, "Shutting down application");
        shutdown_subsystems();
        Logger::shutdown();
        s_instance = nullptr;
    }

    bool Application::initialize_subsystems() {
        m_window_manager = std::make_unique<WindowManager>();
        m_context.register_service(m_window_manager.get());

        m_input_manager = platform::Input::create();
        m_context.register_service(m_input_manager.get());

        m_window_manager->set_input_manager(m_input_manager.get());

        const GameLoopConfig loop_config{.fixed_timestep = m_config.fixed_timestep, .target_fps = m_config.max_fps};
        m_game_loop = std::make_unique<GameLoop>(loop_config);

        STAR_LOG_INFO(LogCategory::Application, "All subsystems initialized successfully");
        return true;
    }

    void Application::shutdown_subsystems() {
        if (m_subsystems_shut_down) {
            return;
        }
        m_subsystems_shut_down = true;

        STAR_LOG_INFO(LogCategory::Application, "Shutting down subsystems");

        on_shutdown();

        for (auto& window : m_app_windows) {
            if (window) {
                window->shutdown();
            }
        }
        m_app_windows.clear();

        if (m_window_manager) {
            m_window_manager->destroy_all();
            m_window_manager.reset();
        }

        if (m_input_manager) {
            m_input_manager.reset();
        }

        m_context.clear();

        m_running = false;
    }

    i32 Application::run() {
        try {
            if (!on_initialize()) {
                STAR_LOG_ERROR(LogCategory::Application, "Application initialization failed");
                return -1;
            }

            m_game_loop->run([this](const f32 fixed_dt) { fixed_update_frame(fixed_dt); },
                             [this](const f32 delta_time) { update_frame(delta_time); },
                             [this](const f32) { render_frame(); },
                             [this] {
                                 if (!m_window_manager->has_open_windows()) {
                                     STAR_LOG_INFO(LogCategory::Application, "All windows closed, shutting down");
                                     shutdown();
                                     return false;
                                 }
                                 return m_running;
                             });
            return 0;
        } catch (const std::exception& e) {
            STAR_LOG_ERROR(LogCategory::Application, "Unhandled exception: {}", e.what());
            return -1;
        } catch (...) {
            STAR_LOG_ERROR(LogCategory::Application, "Unhandled unknown exception");
            return -1;
        }
    }

    void Application::update_frame(const f32 delta_time) {
        m_window_manager->poll_events();

        if (m_input_manager) {
            m_input_manager->update();
        }

        for (const auto& app_window : m_app_windows) {
            if (app_window && app_window->is_open()) {
                app_window->update(delta_time);
            }
        }

        on_update(delta_time);
        on_post_update(delta_time);
    }

    void Application::fixed_update_frame(const f32 fixed_dt) {
        for (const auto& app_window : m_app_windows) {
            if (app_window && app_window->is_open()) {
                app_window->fixed_update(fixed_dt);
            }
        }

        on_fixed_update(fixed_dt);
    }

    void Application::render_frame() const {
        const auto delta_time = m_game_loop->delta_time();

        for (auto& app_window : m_app_windows) {
            if (app_window && app_window->is_open()) {
                app_window->render(delta_time);
            }
        }
    }

    void Application::shutdown() {
        m_running = false;
    }

    platform::Window& Application::main_window() const {
        return *m_window_manager->get_main_window();
    }

    WindowManager& Application::window_manager() const {
        return *m_window_manager;
    }

    platform::Input& Application::input_manager() const {
        return *m_input_manager;
    }
} // namespace star::application
