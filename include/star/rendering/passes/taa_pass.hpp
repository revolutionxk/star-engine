#pragma once

#include "star/rendering/passes/post_pass.hpp"

namespace star::rendering {
    class TaaPass final : public PostPass {
      public:
        TaaPass(graphics::Device& device, resources::ResourceManager& resources, const PostProcessSettings& settings);

        [[nodiscard]] std::string get_name() const override {
            return "Taa";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 211;
        }

        void render(const RenderContext& ctx) override;

      private:
        void resolve_uniforms(graphics::DeviceContext& gpu) override;

        graphics::UniformId m_cur_inv_vp;
        graphics::UniformId m_prev_vp;
        graphics::UniformId m_params;
        graphics::UniformId m_texel;
        graphics::UniformId m_s_current;
        graphics::UniformId m_s_history;
        graphics::UniformId m_s_depth;
        graphics::UniformId m_s_velocity;
    };
} // namespace star::rendering
