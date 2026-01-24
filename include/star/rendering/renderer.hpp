#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "star/core/types.hpp"

namespace star::graphics {
    class Device;
    class DeviceContext;
} // namespace star::graphics

namespace star::scene {
    class Scene;
}

namespace star::systems {
    class RenderSystem;
}

namespace star::resources {
    class ResourceManager;
}

namespace star::application {
    class WindowManager;
} // namespace star::application

namespace star::platform {
    class Window;
    class ImGuiSystem;
} // namespace star::platform

namespace star::rendering {
    class WindowRenderContext;

    class STAR_EXPORT IRenderPass {
      public:
        virtual ~IRenderPass() = default;

        IRenderPass(const IRenderPass&) = delete;
        IRenderPass& operator=(const IRenderPass&) = delete;

        virtual std::string get_name() const = 0;
        virtual u8 get_priority() const = 0;

        virtual void pre_render(f32 delta_time) = 0;

        virtual void render(graphics::DeviceContext& context, const u32 view_id) {
            m_view_id = view_id;
        }

        virtual void post_render(f32 delta_time) = 0;

        /**
         * @brief Reset the render pass
         * Useful for handling window resize events and buffer recreation
         * @param width New width for the render pass (0 means keep current)
         * @param height New height for the render pass (0 means keep current)
         */
        virtual u32 reset(u32 width = 0, u32 height = 0) {
            return m_view_id;
        }

        [[nodiscard]] virtual bool is_enabled() const {
            return m_enabled;
        }

        virtual void set_enabled(const bool enabled) {
            m_enabled = enabled;
        }

      protected:
        IRenderPass() = default;

      private:
        bool m_enabled = true;
        u32 m_view_id = 0;
    };

    class Renderer {
      public:
        Renderer(graphics::Device& device, platform::Window& window, resources::ResourceManager& resource_manager);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        void render_frame(f32 delta_time) const;
        void add_render_pass(std::unique_ptr<IRenderPass> render_pass);
        void remove_render_pass(const char* name);

        template<typename T>
        T* get_render_pass() const {
            for (const auto& pass : m_render_passes) {
                if (auto* casted_pass = dynamic_cast<T*>(pass.get())) {
                    return casted_pass;
                }
            }
            return nullptr;
        }

        [[nodiscard]] systems::RenderSystem* get_render_system() const {
            return m_render_system.get();
        }

        void sort_render_passes();

        void set_active_scene(scene::Scene* scene);

        void reset_render_passes(u32 width, u32 height) const;

      private:
        graphics::Device* m_device;
        platform::Window* m_window;
        resources::ResourceManager* m_resource_manager;
        scene::Scene* m_active_scene = nullptr;

        std::vector<std::unique_ptr<IRenderPass>> m_render_passes;
        std::unique_ptr<systems::RenderSystem> m_render_system;

        bool m_initialized = false;
    };

} // namespace star::rendering
