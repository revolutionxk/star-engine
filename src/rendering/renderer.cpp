#include "star/rendering/renderer.hpp"

#include "../graphics/bgfx/imgui_bgfx_renderer.hpp"
#include "platform/sdl/imgui_sdl3_backend.hpp"
#include "star/core/logger.hpp"
#include "star/graphics/device.hpp"
#include "star/platform/window.hpp"
#include "star/rendering/passes/imgui_render_pass.hpp"
#include "star/rendering/passes/scene_render_pass.hpp"
#include "star/scene/scene.hpp"
#include "star/systems/render_system.hpp"

namespace star::rendering {
    Renderer::Renderer(graphics::Device& device, platform::Window& window, resources::ResourceManager& resource_manager)
        : m_device(&device), m_window(&window), m_resource_manager(&resource_manager) {
        if (m_initialized) {
            STAR_LOG_WARN(LogCategory::Rendering, "Renderer already initialized");
        }

        m_render_system = std::make_unique<systems::RenderSystem>(*m_device);
        m_render_system->set_resource_manager(m_resource_manager);

        auto scene_pass = std::make_unique<SceneRenderPass>(*m_render_system);
        auto imgui_pass = std::make_unique<ImGuiRenderPass>(std::make_unique<platform::sdl::ImGuiSDL3Backend>(),
                                                            std::make_unique<graphics::ImGuiBGFXRenderer>());

        add_render_pass(std::move(scene_pass));
        add_render_pass(std::move(imgui_pass));

        STAR_LOG_INFO(LogCategory::Rendering, "Window rendering setup complete");

        m_initialized = true;
        STAR_LOG_INFO(LogCategory::Rendering, "Renderer initialized successfully");
    }

    Renderer::~Renderer() {
        if (!m_initialized) {
            return;
        }

        STAR_LOG_INFO(LogCategory::Rendering, "Shutting down Renderer");

        for (const auto& pass : m_render_passes) {
            pass->post_render(0.0f);
        }

        m_render_passes.clear();

        m_render_system.reset();
        m_initialized = false;
    }

    void Renderer::render_frame(const f32 delta_time) const {
        if (!m_initialized) {
            return;
        }

        const auto context = m_device->context();

        for (const auto& pass : m_render_passes) {
            if (pass->is_enabled()) {
                pass->pre_render(delta_time);
            }
        }

        context->begin_frame();
        u32 view_id = 0;
        for (const auto& pass : m_render_passes) {
            if (pass->is_enabled()) {
                pass->render(*m_device->context(), view_id++);
            }
        }
        context->end_frame();

        for (const auto& pass : m_render_passes) {
            if (pass->is_enabled()) {
                pass->post_render(delta_time);
            }
        }
    }

    void Renderer::add_render_pass(std::unique_ptr<IRenderPass> render_pass) {
        if (!render_pass) {
            STAR_LOG_WARN(LogCategory::Rendering, "Attempted to add null render pass");
            return;
        }

        auto name = render_pass->get_name();
        STAR_LOG_INFO(LogCategory::Rendering, "Adding render pass: {} (priority: {})", name,
                      render_pass->get_priority());

        m_render_passes.push_back(std::move(render_pass));
        sort_render_passes();
    }

    void Renderer::remove_render_pass(const char* name) {
        const auto it = std::ranges::remove_if(m_render_passes, [name](const std::unique_ptr<IRenderPass>& pass) {
                            return pass->get_name() == name;
                        }).begin();

        if (it != m_render_passes.end()) {
            STAR_LOG_INFO(LogCategory::Rendering, "Removing render pass: {}", name);
            m_render_passes.erase(it, m_render_passes.end());
        } else {
            STAR_LOG_WARN(LogCategory::Rendering, "Render pass not found: {}", name);
        }
    }

    void Renderer::sort_render_passes() {
        std::ranges::sort(m_render_passes,
                          [](const std::unique_ptr<IRenderPass>& a, const std::unique_ptr<IRenderPass>& b) {
                              return a->get_priority() < b->get_priority();
                          });

        STAR_LOG_DEBUG(LogCategory::Rendering, "Render pass execution order:");
        for (const auto& pass : m_render_passes) {
            STAR_LOG_DEBUG(LogCategory::Rendering, "  - {} (priority: {})", pass->get_name(), pass->get_priority());
        }
    }

    void Renderer::set_active_scene(scene::Scene* scene) {
        m_active_scene = scene;

        if (auto* scene_pass = get_render_pass<SceneRenderPass>()) {
            scene_pass->set_scene(scene);
            STAR_LOG_DEBUG(LogCategory::Rendering, "Scene '{}' set on SceneRenderPass",
                           scene ? scene->name() : "nullptr");
        }
    }

    void Renderer::reset_render_passes(const u32 width, const u32 height) const {
        if (!m_initialized) {
            STAR_LOG_WARN(LogCategory::Rendering, "Cannot reset render passes - renderer not initialized");
            return;
        }

        STAR_LOG_INFO(LogCategory::Rendering, "Resetting render passes with size: {}x{}", width, height);

        for (const auto& pass : m_render_passes) {
            if (pass) {
                const auto view_id = pass->reset(width, height);
                const auto context = m_device->context();
                context->set_view_clear(view_id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
                context->set_view_rect(view_id, 0, 0, width, height);
            }
        }

        STAR_LOG_INFO(LogCategory::Rendering, "All render passes reset successfully");
    }
} // namespace star::rendering
