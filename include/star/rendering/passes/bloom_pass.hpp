#pragma once

#include "star/rendering/passes/post_pass.hpp"

namespace star::rendering {
    class BloomPass final : public PostPass {
      public:
        BloomPass(graphics::Device& device, resources::ResourceManager& resources,
                  const PostProcessSettings& settings);

        [[nodiscard]] std::string get_name() const override {
            return "Bloom";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 214;
        }

        void render(const RenderContext& ctx) override;

      private:
        void resolve_uniforms(graphics::DeviceContext& gpu) override;

        graphics::ResourceHandle<graphics::Shader> m_blur_shader;

        graphics::UniformId m_bloom_params;
        graphics::UniformId m_blur_params;
        graphics::UniformId m_s_hdr;
        graphics::UniformId m_s_src;
    };
} // namespace star::rendering
