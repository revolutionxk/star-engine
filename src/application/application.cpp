#include "star/application/application.hpp"

namespace star::application {
    Application* Application::s_instance = nullptr;

    Application::Application(ApplicationConfig config) : m_config(std::move(config)) {
        Logger::initialize(LogLevel::Debug, LogLevel::Trace, "logs/star_engine.log");

        STAR_CORE_ASSERT(!s_instance, "Application already exists!");

        s_instance = this;

        m_window = Window::create_platform_window();
        m_window->create(m_config.window);

        STAR_CORE_INFO("Application created: {}", m_config.title);
    }

    Application::~Application() {
        STAR_CORE_INFO("Application destructor called");

        m_window->destroy();

        shutdown();
        Logger::shutdown();
        s_instance = nullptr;
    }

    i32 Application::run() {
        if (!on_initialize()) {
            return -1;
        }

        m_running = true;

        while (m_running) {
            f32 delta_time = 0.016f;
            for (const auto& layer : m_layer_stack) {
                layer->update(delta_time);
            }

            m_window->pool_events();

            on_update(delta_time);

            for (auto& layer : m_layer_stack) {
                layer->render();
            }
        }

        return 0;
    }

    void Application::shutdown() {
        on_shutdown();

        m_running = false;
    }

    void Application::push_layer(std::unique_ptr<Layer> layer) {
        layer->on_attach();
        if (!layer->initialize()) {
            return;
        }
        m_layer_stack.push_layer(std::move(layer));
    }

    void Application::push_overlay(std::unique_ptr<Layer> overlay) {
        overlay->on_attach();
        if (!overlay->initialize()) {
            return;
        }
        m_layer_stack.push_overlay(std::move(overlay));
    }
} // namespace star::application
