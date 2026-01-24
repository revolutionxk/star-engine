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

namespace star::systems {
    class RenderSystem {
      public:
        explicit RenderSystem(graphics::Device& device);
        ~RenderSystem();

        void set_resource_manager(resources::ResourceManager* resource_manager) {
            m_resource_manager = resource_manager;
        }

        void render(scene::Scene& scene, graphics::DeviceContext& context, u32 view_id);
        void update(f32 delta_time);

        const rendering::RenderQueue& render_queue() const {
            return m_render_queue;
        }

        void set_viewport_size(const u32 width, const u32 height) {
            m_viewport_width = width;
            m_viewport_height = height;
        }

      private:
        void collect_renderables(scene::Scene& scene);
        void setup_camera(scene::Scene& scene);
        void execute_render_queue(graphics::DeviceContext& context, u32 view_id);

        graphics::Device& m_device;
        resources::ResourceManager* m_resource_manager{nullptr};
        rendering::RenderQueue m_render_queue;

        Matrix4 m_view_matrix;
        Matrix4 m_projection_matrix;
        Vector3 m_camera_position;
        bool m_has_camera{false};

        u32 m_viewport_width{1280};
        u32 m_viewport_height{720};
    };
} // namespace star::systems
