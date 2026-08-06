#pragma once

#include <array>

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
        static constexpr u32 MAX_MIPS = 6;
        static constexpr u32 MIN_MIP_SIZE = 8;

        void resolve_uniforms(graphics::DeviceContext& gpu) override;

        graphics::ResourceHandle<graphics::Shader> m_upsample_shader;

        graphics::UniformId m_bloom_params;
        graphics::UniformId m_bloom_texel;
        graphics::UniformId m_s_src;
        graphics::UniformId m_s_prev;
    };
} // namespace star::rendering
