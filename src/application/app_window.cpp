#include "star/application/app_window.hpp"

#include <utility>

#include "star/application/application.hpp"
#include "star/graphics/device.hpp"
#include "star/platform/window.hpp"
#include "star/rendering/passes/imgui_render_pass.hpp"
#include "star/rendering/renderer.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/scene/scene_manager.hpp"

namespace star::application {
    AppWindow::AppWindow(const std::string_view title) : m_configuration{.title = std::string(title)} {}

    AppWindow::AppWindow(const std::string_view title, const platform::VideoMode& video_mode)
        : m_configuration{.title = std::string(title), .video_mode = video_mode} {}

    AppWindow::AppWindow(platform::WindowConfiguration config) : m_configuration(std::move(config)) {}

    AppWindow::~AppWindow() {
        if (m_initialized) {
            AppWindow::shutdown();
        }
    }

    bool AppWindow::initialize() {
        STAR_LOG_INFO(LogCategory::Application, "Initializing window: {}", m_configuration.title);

        if (!initialize_subsystems()) {
            STAR_LOG_ERROR(LogCategory::Application, "Failed to initialize window subsystems");
            return false;
        }

        if (!on_initialize()) {
            STAR_LOG_ERROR(LogCategory::Application, "Window custom initialization failed");
            return false;
        }

        m_initialized = true;
        STAR_LOG_INFO(LogCategory::Application, "Window initialized successfully: {}", m_configuration.title);
        return true;
    }

    void AppWindow::shutdown() {
        STAR_LOG_INFO(LogCategory::Application, "Shutting down window: {}", m_configuration.title);

        on_shutdown();
        shutdown_subsystems();

        m_initialized = false;
    }

    bool AppWindow::initialize_subsystems() {
        if (!Application::has_instance()) {
            STAR_LOG_ERROR(LogCategory::Application, "Application instance not available");
            return false;
        }

        const auto& app = Application::instance();

        auto& window_manager = app.window_manager();

        m_window_id = window_manager.create_window(m_configuration);
        if (m_window_id == INVALID_WINDOW_ID) {
            STAR_LOG_ERROR(LogCategory::Platform, "Failed to create platform window");
            return false;
        }

        m_window = window_manager.get_window(m_window_id);
        if (!m_window) {
            STAR_LOG_ERROR(LogCategory::Platform, "Failed to retrieve created window");
            return false;
        }

        const auto platform_data = m_window->platform_data();
        const auto size = m_window->size();

        const graphics::GraphicsDeviceConfig device_config{
            .api = app.config().graphics_api, .platform = platform_data, .window_size = size};

        m_device = graphics::Device::create(device_config);
        if (!m_device) {
            STAR_LOG_ERROR(LogCategory::Application, "Failed to create graphics device");
            return false;
        }

        m_resource_manager = std::make_shared<resources::ResourceManager>(*m_device);
        m_renderer = std::make_unique<rendering::Renderer>(*m_device, *m_window, *m_resource_manager);
        m_scene_manager = std::make_unique<scene::SceneManager>();
        m_layer_stack = std::make_unique<LayerStack>();

        m_window->set_resize_callback([this](const u32 width, const u32 height) { on_window_resize(width, height); });

        m_scene_manager->set_scene_changed_callback([this](scene::Scene* scene) {
            if (m_renderer) {
                m_renderer->set_active_scene(scene);
            }
        });

        if (const auto imgui_render_pass = m_renderer->get_render_pass<rendering::ImGuiRenderPass>()) {
            imgui_render_pass->set_window(m_window);
            imgui_render_pass->add_imgui_callback([this] {
                if (m_layer_stack) {
                    for (const auto& layer : *m_layer_stack) {
                        layer->on_imgui_render();
                    }
                }
            });
        }

        STAR_LOG_INFO(LogCategory::Application, "Window subsystems initialized");
        return true;
    }

    void AppWindow::shutdown_subsystems() {
        if (m_layer_stack) {
            m_layer_stack->clear();
            m_layer_stack.reset();
        }

        if (m_scene_manager) {
            m_scene_manager.reset();
        }
        m_renderer.reset();

        m_window = nullptr;

        if (m_window_id != INVALID_WINDOW_ID && Application::has_instance()) {
            auto& window_manager = Application::instance().window_manager();
            window_manager.destroy_window(m_window_id);
            m_window_id = INVALID_WINDOW_ID;
        }

        m_device = nullptr;
        m_resource_manager = nullptr;

        STAR_LOG_INFO(LogCategory::Application, "Window subsystems shut down");
    }

    void AppWindow::update(const f32 delta_time) {
        if (!m_initialized || !m_window) {
            return;
        }

        m_window->pool_events();

        on_update(delta_time);

        if (m_layer_stack) {
            for (const auto& layer : *m_layer_stack) {
                layer->update(delta_time);
            }
        }
    }

    void AppWindow::render(const f32 delta_time) {
        if (!m_initialized || !m_window) {
            return;
        }

        m_renderer->render_frame(delta_time);

        on_render();

        if (m_layer_stack) {
            for (const auto& layer : *m_layer_stack) {
                layer->render();
            }
        }
    }

    void AppWindow::push_layer(std::unique_ptr<Layer> layer) {
        if (!m_layer_stack) {
            STAR_LOG_WARN(LogCategory::Application, "Cannot push layer - layer stack not initialized");
            return;
        }

        if (!layer->initialize()) {
            STAR_LOG_ERROR(LogCategory::Application, "Failed to initialize layer");
            return;
        }

        m_layer_stack->push_layer(std::move(layer));
        STAR_LOG_INFO(LogCategory::Application, "Layer pushed to window: {}", m_configuration.title);
    }

    void AppWindow::push_overlay(std::unique_ptr<Layer> overlay) {
        if (!m_layer_stack) {
            STAR_LOG_WARN(LogCategory::Application, "Cannot push overlay - layer stack not initialized");
            return;
        }

        if (overlay->initialize()) {
            m_layer_stack->push_overlay(std::move(overlay));
            STAR_LOG_INFO(LogCategory::Application, "Overlay pushed to window: {}", m_configuration.title);
        } else {
            STAR_LOG_ERROR(LogCategory::Application, "Failed to initialize overlay");
        }
    }

    void AppWindow::pop_layer(Layer* layer) const {
        if (m_layer_stack) {
            m_layer_stack->pop_layer(layer);
        }
    }

    void AppWindow::pop_overlay(Layer* overlay) const {
        if (m_layer_stack) {
            m_layer_stack->pop_overlay(overlay);
        }
    }

    void AppWindow::show() {
        if (m_window) {
            STAR_LOG_INFO(LogCategory::Application, "Window show requested: {}", m_configuration.title);
        }
    }

    void AppWindow::hide() {
        if (m_window) {
            STAR_LOG_INFO(LogCategory::Application, "Window hide requested: {}", m_configuration.title);
        }
    }

    void AppWindow::close() const {
        if (!m_window)
            return;

        m_window->destroy();
    }

    void AppWindow::focus() {
        if (m_window) {
            STAR_LOG_INFO(LogCategory::Application, "Window focus requested: {}", m_configuration.title);
        }
    }

    bool AppWindow::is_open() const {
        return m_window && m_window->is_opened();
    }

    bool AppWindow::is_focused() const {
        return m_window && m_window->is_opened();
    }

    bool AppWindow::should_close() const {
        return !is_open();
    }

    Vector2 AppWindow::size() const {
        if (!m_window) {
            return Vector2::zero();
        }

        return m_window->size();
    }

    void AppWindow::on_window_resize(const u32 width, const u32 height) const {
        if (const auto context = m_device->context()) {
            context->resize(width, height);
        }

        if (m_renderer) {
            m_renderer->reset_render_passes(width, height);
        }
    }
} // namespace star::application
