#pragma once

#include "star/rendering/passes/post_pass.hpp"

namespace star::rendering {
    class SsrPass final : public PostPass {
      public:
        SsrPass(graphics::Device& device, resources::ResourceManager& resources, const PostProcessSettings& settings);

        [[nodiscard]] std::string get_name() const override {
            return "Ssr";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 208;
        }

        void render(const RenderContext& ctx) override;

      private:
        void resolve_uniforms(graphics::DeviceContext& gpu) override;

        graphics::UniformId m_proj;
        graphics::UniformId m_inv_proj;
        graphics::UniformId m_params;
        graphics::UniformId m_texel;
        graphics::UniformId m_s_hdr;
        graphics::UniformId m_s_depth;
        graphics::UniformId m_s_scene;
        graphics::UniformId m_s_gbuffer;
    };
} // namespace star::rendering
