#include "star/application/application.hpp"

#include "star/graphics/device.hpp"

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

        if (args.has_flag("fullscreen")) {
            m_config.main_window.mode = WindowMode::Fullscreen;
            STAR_LOG_INFO(LogCategory::Application, "Starting in fullscreen mode");
        }

        if (const WindowId main_window_id = m_window_manager.create_window(m_config.main_window, true);
            main_window_id == INVALID_WINDOW_ID) {
            STAR_LOG_ERROR(LogCategory::Application, "Failed to create main window");
            return;
        }

        auto* main_window = m_window_manager.get_main_window();
        if (!main_window) {
            STAR_ASSERT(false, "Main window is null after creation");
            return;
        }
        main_window->set_title(m_config.title);

        const auto size = main_window->size();
        const auto platform_data = main_window->platform_data();

        graphics::GraphicsDeviceConfig device_config{
            .api = m_config.graphics_api, .platform = platform_data, .window_size = size};

        m_device = graphics::Device::create(m_config.graphics_api);
        if (!m_device) {
            STAR_LOG_ERROR(LogCategory::Application, "Failed to create graphics device");
            return;
        }

        if (!m_device->initialize(device_config)) {
            STAR_LOG_ERROR(LogCategory::Application, "Graphics device initialization failed");
            return;
        }

        auto context = m_device->create_context();
        if (!context) {
            STAR_LOG_ERROR(LogCategory::Application, "Failed to create device context for main window");
            return;
        }

        main_window->set_device_context(std::move(context));

        STAR_LOG_INFO(LogCategory::Application, "Application created: {}", m_config.title);
    }

    Application::~Application() {
        STAR_LOG_INFO(LogCategory::Application, "Shutting down application");

        shutdown();

        Logger::shutdown();
        s_instance = nullptr;
    }

    i32 Application::run() {
        if (!on_initialize()) {
            STAR_LOG_ERROR(LogCategory::Application, "Application initialization failed");
            return -1;
        }

        m_running = true;

        STAR_LOG_INFO(LogCategory::Application, "Entering main loop");

        while (m_running) {
            const f32 delta_time = m_config.fixed_timestep;

            if (!m_window_manager.has_open_windows()) {
                STAR_LOG_INFO(LogCategory::Application, "All windows closed, shutting down");
                shutdown();
                break;
            }

            for (const auto& layer : m_layer_stack) {
                layer->update(delta_time);
            }

            m_window_manager.poll_events();
            on_update(delta_time);

            for (const auto& layer : m_layer_stack) {
                layer->render();
            }

            on_post_update(delta_time);
        }

        STAR_LOG_INFO(LogCategory::Application, "Main loop exited");
        return 0;
    }

    void Application::shutdown() {
        if (!m_running) {
            return;
        }

        STAR_LOG_INFO(LogCategory::Application, "Application shutdown requested");

        on_shutdown();

        m_window_manager.destroy_all();
        m_running = false;
    }

    void Application::push_layer(std::unique_ptr<Layer> layer) {
        layer->on_attach();
        if (!layer->initialize()) {
            STAR_LOG_ERROR(LogCategory::Application, "Failed to initialize layer");
            return;
        }
        m_layer_stack.push_layer(std::move(layer));
    }

    void Application::push_overlay(std::unique_ptr<Layer> overlay) {
        overlay->on_attach();
        if (!overlay->initialize()) {
            STAR_LOG_ERROR(LogCategory::Application, "Failed to initialize overlay");
            return;
        }
        m_layer_stack.push_overlay(std::move(overlay));
    }
} // namespace star::application
