#pragma once
#include "star/core/types.hpp"
#include "star/rendering/light_environment.hpp"
#include "star/rendering/render_queue.hpp"

namespace star::graphics {
    class Device;
    class DeviceContext;
} // namespace star::graphics

namespace star::scene {
    class Scene;
} // namespace star::scene

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::rendering {
    class Viewport;
} // namespace star::rendering

namespace star::systems {
    class RenderSystem {
      public:
        RenderSystem(graphics::Device& device, resources::ResourceManager& resource_manager);
        ~RenderSystem();

        void render(scene::Scene& scene, graphics::DeviceContext& context, u32 view_id,
                    rendering::Viewport* viewport = nullptr);
        void update(f32 delta_time);

        [[nodiscard]] const rendering::RenderQueue& render_queue() const {
            return m_render_queue;
        }

      private:
        void collect_renderables(scene::Scene& scene, const rendering::Viewport* viewport);
        void setup_camera(scene::Scene& scene, rendering::Viewport* viewport);
        void collect_lights(scene::Scene& scene);
        void submit_lighting(graphics::DeviceContext& context) const;
        void execute_render_queue(graphics::DeviceContext& context, u32 view_id, const rendering::Viewport* viewport);

        graphics::Device& m_device;
        resources::ResourceManager& m_resource_manager;
        rendering::RenderQueue m_render_queue;
        rendering::LightEnvironment m_light_env;
    };
} // namespace star::systems
