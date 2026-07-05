#pragma once
#include "star/core/types.hpp"
#include "star/rendering/light_environment.hpp"
#include "star/rendering/render_queue.hpp"
#include "star/rendering/sky/atmosphere_solver.hpp"

namespace star::graphics {
    class Device;
    class DeviceContext;
} // namespace star::graphics

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::rendering {
    class Viewport;
    struct RenderScene;
    struct CameraSnapshot;
} // namespace star::rendering

namespace star::systems {
    class RenderSystem {
      public:
        RenderSystem(graphics::Device& device, resources::ResourceManager& resource_manager);
        ~RenderSystem();

        void render(const rendering::RenderScene& scene, graphics::DeviceContext& context, u32 view_id,
                    rendering::Viewport* viewport = nullptr);

        void apply_atmospheric_lighting(const rendering::AtmosphericLighting& lighting) {
            m_atmospheric = lighting;
        }
        void clear_atmospheric_lighting() {
            m_atmospheric.valid = false;
        }

        [[nodiscard]] const rendering::RenderQueue& render_queue() const {
            return m_render_queue;
        }

      private:
        void collect_renderables(const rendering::RenderScene& scene, const rendering::Viewport* viewport);
        void collect_lights(const rendering::RenderScene& scene);
        void submit_lighting(graphics::DeviceContext& context) const;
        void execute_render_queue(graphics::DeviceContext& context, u32 view_id, const rendering::Viewport* viewport) const;

        graphics::Device& m_device;
        resources::ResourceManager& m_resource_manager;
        rendering::RenderQueue m_render_queue;
        rendering::LightEnvironment m_light_env;
        rendering::AtmosphericLighting m_atmospheric;
    };
} // namespace star::systems
