#pragma once

#include "star/core/types.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/math/math.hpp"
#include "star/rendering/render_target.hpp"
#include "star/rendering/renderer.hpp"

namespace star::graphics {
    class Device;
    struct Shader;
} // namespace star::graphics

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::systems {
    class RenderSystem;
} // namespace star::systems

namespace star::rendering {
    class ShadowPass final : public IRenderPass {
      public:
        ShadowPass(graphics::Device& device, resources::ResourceManager& resources,
                   systems::RenderSystem& render_system);
        ~ShadowPass() override;

        [[nodiscard]] std::string get_name() const override {
            return "ShadowPass";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 50;
        }

        void render(const RenderContext& ctx) override;

      private:
        static constexpr u32 SHADOW_MAP_SIZE = 2048;
        static constexpr f32 ORTHO_HALF = 30.0f;
        static constexpr f32 LIGHT_DISTANCE = 60.0f;
        static constexpr f32 SHADOW_NEAR = 5.0f;
        static constexpr f32 SHADOW_FAR = 120.0f;
        static constexpr f32 SHADOW_BIAS = 0.002f;

        static bool find_sun_direction(const RenderScene& scene, Vector3& out_travel_dir);
        static void compute_light_matrices(const Vector3& sun_travel_dir, const Vector3& center, bool homogeneous_depth,
                                           Matrix4& out_view, Matrix4& out_proj);

        graphics::Device& m_device;
        resources::ResourceManager& m_resources;
        systems::RenderSystem& m_render_system;

        graphics::ResourceHandle<graphics::Shader> m_shadow_shader;
        RenderTarget m_target;
    };
} // namespace star::rendering
