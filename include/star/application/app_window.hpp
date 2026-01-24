#pragma once

#include <memory>
#include <string>

#include "layer_stack.hpp"
#include "star/core/lifecycle.hpp"
#include "star/core/types.hpp"
#include "star/platform/window.hpp"
#include "star/platform/window_config.hpp"
#include "window_manager.hpp"

namespace star::graphics {
    class Device;
    class DeviceContext;
} // namespace star::graphics

namespace star::rendering {
    class Renderer;
} // namespace star::rendering

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::scene {
    class SceneManager;
} // namespace star::scene

namespace star::application {
    class STAR_EXPORT AppWindow : public IInitializable {
      public:
        explicit AppWindow(std::string_view title);
        explicit AppWindow(std::string_view title, const platform::VideoMode& video_mode);
        explicit AppWindow(platform::WindowConfiguration config);
        ~AppWindow() override;

        bool initialize() override;
        void shutdown() override;
        void update(f32 delta_time);
        void render(f32 delta_time);

        void push_layer(std::unique_ptr<Layer> layer);
        void push_overlay(std::unique_ptr<Layer> overlay);
        void pop_layer(Layer* layer) const;
        void pop_overlay(Layer* overlay) const;

        void show();
        void hide();
        void close() const;
        void focus();

        [[nodiscard]] bool is_open() const;
        [[nodiscard]] bool is_focused() const;
        [[nodiscard]] bool should_close() const;

        [[nodiscard]] platform::Window& window() const {
            return *m_window;
        }

        [[nodiscard]] graphics::Device& device() const {
            return *m_device;
        }

        [[nodiscard]] rendering::Renderer& renderer() const {
            return *m_renderer;
        }

        [[nodiscard]] scene::SceneManager& scene_manager() const {
            return *m_scene_manager;
        }

        [[nodiscard]] resources::ResourceManager& resources() const {
            return *m_resource_manager;
        }

        [[nodiscard]] const std::string& title() const {
            return m_configuration.title;
        }

        [[nodiscard]] Vector2 size() const;

        void on_window_resize(u32 width, u32 height) const;

      protected:
        virtual bool on_initialize() {
            return true;
        }

        virtual void on_shutdown() {}

        virtual void on_update(f32 delta_time) {}

        virtual void on_render() {}

      private:
        bool initialize_subsystems();
        void shutdown_subsystems();

        platform::WindowConfiguration m_configuration;
        WindowId m_window_id{INVALID_WINDOW_ID};
        platform::Window* m_window{nullptr};

        std::unique_ptr<graphics::Device> m_device{nullptr};
        std::unique_ptr<rendering::Renderer> m_renderer;
        std::unique_ptr<resources::ResourceManager> m_resource_manager;

        std::unique_ptr<scene::SceneManager> m_scene_manager;

        std::unique_ptr<LayerStack> m_layer_stack;

        bool m_initialized{false};
    };
} // namespace star::application
