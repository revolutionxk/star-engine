#pragma once

#include "star/rendering/passes/post_pass.hpp"

namespace star::rendering {
    class AutoExposurePass final : public PostPass {
      public:
        AutoExposurePass(graphics::Device& device, resources::ResourceManager& resources,
                         const PostProcessSettings& settings);

        [[nodiscard]] std::string get_name() const override {
            return "AutoExposure";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 213;
        }

        void render(const RenderContext& ctx) override;

      private:
        static constexpr u32 LUM_SIZE = 64;
        static constexpr u32 REDUCE_FACTOR = 4;
        static constexpr u32 REDUCE_STEPS = 2;

        void resolve_uniforms(graphics::DeviceContext& gpu) override;

        graphics::ResourceHandle<graphics::Shader> m_exposure_shader;

        graphics::UniformId m_lum_params;
        graphics::UniformId m_exposure_params;
        graphics::UniformId m_s_src;
        graphics::UniformId m_s_lum;
        graphics::UniformId m_s_prev;
    };
} // namespace star::rendering
