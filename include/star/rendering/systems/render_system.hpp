#pragma once
#include <unordered_map>

#include "star/core/types.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/math/matrix.hpp"
#include "star/rendering/light_environment.hpp"
#include "star/rendering/render_queue.hpp"
#include "star/rendering/sky/atmosphere_solver.hpp"

namespace star::graphics {
    class Device;
    class DeviceContext;
    struct Texture;
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

        struct ShadowState {
            graphics::ResourceHandle<graphics::Texture> map{};
            Matrix4 light_view_proj{Matrix4::identity()};
            bool enabled{false};
            f32 bias{0.0025f};
            f32 texel_size{1.0f / 2048.0f};
            bool origin_bottom_left{false};
        };

        void set_shadow(const ShadowState& shadow) {
            m_shadow = shadow;
        }

        struct EnvironmentState {
            graphics::ResourceHandle<graphics::Texture> map{};
            f32 max_mip{0.0f};
            f32 intensity{1.0f};
        };

        void set_environment(const EnvironmentState& environment) {
            m_environment = environment;
        }

        [[nodiscard]] const EnvironmentState& environment() const {
            return m_environment;
        }

        [[nodiscard]] const rendering::RenderQueue& render_queue() const {
            return m_render_queue;
        }

      private:
        void collect_renderables(const rendering::RenderScene& scene, const rendering::Viewport* viewport);
        void collect_lights(const rendering::RenderScene& scene);
        void submit_lighting(graphics::DeviceContext& context) const;
        void execute_render_queue(graphics::DeviceContext& context, u32 view_id,
                                  const rendering::Viewport* viewport) const;

        graphics::Device& m_device;
        resources::ResourceManager& m_resource_manager;
        rendering::RenderQueue m_render_queue;
        rendering::LightEnvironment m_light_env;
        rendering::AtmosphericLighting m_atmospheric;
        ShadowState m_shadow;
        EnvironmentState m_environment;
        mutable std::unordered_map<u64, Matrix4> m_prev_models;
    };
} // namespace star::systems
