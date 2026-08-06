#pragma once

#include "star/rendering/passes/post_pass.hpp"

namespace star::rendering {
    class SsaoPass final : public PostPass {
      public:
        SsaoPass(graphics::Device& device, resources::ResourceManager& resources,
                 const PostProcessSettings& settings);

        [[nodiscard]] std::string get_name() const override {
            return "Ssao";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 205;
        }

        void render(const RenderContext& ctx) override;

      private:
        void resolve_uniforms(graphics::DeviceContext& gpu) override;

        graphics::ResourceHandle<graphics::Shader> m_blur_shader;

        graphics::UniformId m_inv_proj;
        graphics::UniformId m_proj;
        graphics::UniformId m_params;
        graphics::UniformId m_texel;
        graphics::UniformId m_blur_params;
        graphics::UniformId m_s_depth;
        graphics::UniformId m_s_gbuffer;
        graphics::UniformId m_s_ao;
    };
} // namespace star::rendering
