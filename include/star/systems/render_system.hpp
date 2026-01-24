#pragma once
#include "star/core/types.hpp"
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
        explicit RenderSystem(graphics::Device& device);
        ~RenderSystem();

        void set_resource_manager(resources::ResourceManager* resource_manager) {
            m_resource_manager = resource_manager;
        }

        void render(scene::Scene& scene, graphics::DeviceContext& context, u32 view_id,
                    rendering::Viewport* viewport = nullptr);
        void update(f32 delta_time);

        const rendering::RenderQueue& render_queue() const {
            return m_render_queue;
        }

      private:
        void collect_renderables(scene::Scene& scene, const rendering::Viewport* viewport);
        static void setup_camera(scene::Scene& scene, rendering::Viewport* viewport);
        void execute_render_queue(graphics::DeviceContext& context, u32 view_id,
                                  const rendering::Viewport* viewport) const;

        graphics::Device& m_device;
        resources::ResourceManager* m_resource_manager{nullptr};
        rendering::RenderQueue m_render_queue;
    };
} // namespace star::systems
