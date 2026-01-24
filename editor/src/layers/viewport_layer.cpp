#include "viewport_layer.hpp"

#include "star/application/application.hpp"
#include "star/graphics/device.hpp"
#include "star/graphics/device_context.hpp"
#include "star/rendering/render_target.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/scene/camera3d.hpp"
#include "star/scene/mesh_instance3d.hpp"
#include "star/scene/scene_manager.hpp"
#include "star/systems/render_system.hpp"

namespace star::editor {
    ViewportLayer::ViewportLayer(EditorWindow* editor_window)
        : Layer("ViewportLayer"), m_editor_window(editor_window) {}

    bool ViewportLayer::initialize() {
        STAR_LOG_INFO(LogCategory::Editor, "Initializing viewport layer");

        const auto& app = application::Application::instance();
        auto& device = m_editor_window->device();

        m_render_system = new systems::RenderSystem(device);
        m_render_system->set_resource_manager(&m_editor_window->resources());
        m_render_system->set_viewport_size(m_viewport_width, m_viewport_height);

        setup_render_target();

        setup_scene_content();

        return true;
    }

    void ViewportLayer::shutdown() {
        STAR_LOG_INFO(LogCategory::Editor, "Shutting down viewport layer");

        if (m_scene_render_target) {
            m_scene_render_target->destroy();
            m_scene_render_target.reset();
        }

        if (m_render_system) {
            delete m_render_system;
            m_render_system = nullptr;
        }
    }

    void ViewportLayer::setup_scene_content() {
        STAR_LOG_INFO(LogCategory::Editor, "Viewport scene content ready");
    }

    void ViewportLayer::setup_render_target() {
        auto& device = m_editor_window->device();
        m_scene_render_target = std::make_unique<rendering::RenderTarget>();

        const bool success = m_scene_render_target->create(&device, m_viewport_width, m_viewport_height,
                                                           graphics::TextureDescriptor::Format::RGBA8,
                                                           true // has_depth
        );

        if (success) {
            STAR_LOG_INFO(LogCategory::Editor, "Scene render target created: {}x{}", m_viewport_width,
                          m_viewport_height);
        } else {
            STAR_LOG_ERROR(LogCategory::Editor, "Failed to create scene render target");
        }
    }

    void ViewportLayer::update(const f32 delta_time) {
        if (m_render_system) {
            m_render_system->update(delta_time);
        }
    }

    void ViewportLayer::render() {
        render_scene_to_texture();
    }

    void ViewportLayer::render_scene_to_texture() {
        if (!m_scene_render_target || !m_scene_render_target->is_valid()) {
            return;
        }
    }
} // namespace star::editor
