#pragma once

#include <algorithm>
#include <memory>
#include <string_view>
#include <vector>

#include "star/core/types.hpp"
#include "star/rendering/frame_context.hpp"
#include "star/rendering/render_graph.hpp"

namespace star::graphics {
    class Device;
    class DeviceContext;
} // namespace star::graphics

namespace star::systems {
    class RenderSystem;
}

namespace star::rendering {
    class DebugRenderer;
    struct RenderScene;
    class Viewport;
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

        virtual std::vector<std::string_view> dependencies() const {
            return {};
        }

        virtual void pre_render(const FrameContext& /*frame*/) {}

        virtual void render(const RenderContext& ctx) {
            m_view_id = ctx.view_id;
        }

        virtual void post_render(const FrameContext& /*frame*/) {}

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

        void render_frame(f32 delta_time);

        FrameContext make_frame_context(f32 delta_time);

        void pre_render_passes(const FrameContext& frame);
        void submit_passes(const FrameContext& frame);
        void post_render_passes(const FrameContext& frame);

        void add_render_pass(std::unique_ptr<IRenderPass> render_pass);
        void remove_render_pass(const char* name);

        template<typename T>
        T* get_render_pass() const {
            return m_graph.find_pass_of_type<T>();
        }

        [[nodiscard]] systems::RenderSystem* get_render_system() const {
            return m_render_system.get();
        }

        [[nodiscard]] DebugRenderer* debug_renderer() const {
            return m_debug_renderer.get();
        }

        [[nodiscard]] RenderGraph& graph() noexcept {
            return m_graph;
        }

        void set_render_scene(const RenderScene* scene) noexcept {
            m_active_scene = scene;
        }

        void set_active_viewport(Viewport* viewport) noexcept {
            m_active_viewport = viewport;
        }

        [[nodiscard]] Viewport* active_viewport() const noexcept {
            return m_active_viewport;
        }

        [[nodiscard]] const RenderScene* active_scene() const noexcept {
            return m_active_scene;
        }

        void reset_render_passes(u32 width, u32 height);

      private:
        graphics::Device* m_device;
        platform::Window* m_window;
        resources::ResourceManager* m_resource_manager;

        RenderGraph m_graph;
        std::unique_ptr<systems::RenderSystem> m_render_system;
        std::unique_ptr<DebugRenderer> m_debug_renderer;

        const RenderScene* m_active_scene{nullptr};
        Viewport* m_active_viewport{nullptr};
        u64 m_frame_index{0};

        bool m_initialized = false;
    };

} // namespace star::rendering
