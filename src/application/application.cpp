#include "star/application/application.hpp"

#include <utility>

#include "star/core/assert.hpp"

namespace star::application {
    Application* Application::s_instance = nullptr;

    Application::Application(ApplicationConfig config) : m_config(std::move(config)) {
        STAR_ASSERT(!s_instance, "Application already exists!");
        s_instance = this;
    }

    Application::~Application() {
        shutdown();
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
