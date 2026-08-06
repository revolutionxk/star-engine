#pragma once

#include "star/core/types.hpp"
#include "star/graphics/device_context.hpp"
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
    class IblPass final : public IRenderPass {
      public:
        IblPass(graphics::Device& device, resources::ResourceManager& resources,
                systems::RenderSystem& render_system);
        ~IblPass() override;

        [[nodiscard]] std::string get_name() const override {
            return "IblPass";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 10;
        }

        void render(const RenderContext& ctx) override;

      private:
        static constexpr u32 BRDF_SIZE = 256;
        static constexpr u32 IRRADIANCE_WIDTH = 128;
        static constexpr u32 IRRADIANCE_HEIGHT = 64;
        static constexpr u32 SPECULAR_WIDTH = 512;
        static constexpr u32 SPECULAR_STRIP_HEIGHT = 256;
        static constexpr u32 SPECULAR_LEVELS = 6;

        bool ensure_ready(graphics::DeviceContext& gpu);

        void bake_brdf(const RenderContext& ctx);
        bool bake_environment(const RenderContext& ctx, graphics::ResourceHandle<graphics::Texture> source,
                              u32 source_width, u32 source_height);
        void release_environment();

        void draw(graphics::DeviceContext& gpu, u32 view_id, graphics::ResourceHandle<graphics::Shader> shader) const;

        graphics::Device& m_device;
        resources::ResourceManager& m_resources;
        systems::RenderSystem& m_render_system;

        graphics::ResourceHandle<graphics::Shader> m_brdf_shader;
        graphics::ResourceHandle<graphics::Shader> m_irradiance_shader;
        graphics::ResourceHandle<graphics::Shader> m_prefilter_shader;

        RenderTarget m_brdf;
        RenderTarget m_irradiance;
        RenderTarget m_specular;

        graphics::UniformId m_post_params;
        graphics::UniformId m_bake_params;
        graphics::UniformId m_s_src;

        graphics::ResourceHandle<graphics::Texture> m_baked_source;
        bool m_uniforms_resolved = false;
        bool m_brdf_ready = false;
    };
} // namespace star::rendering
