#pragma once

#include "star/rendering/passes/post_pass.hpp"

namespace star::rendering {
    class TonemapPass final : public PostPass {
      public:
        TonemapPass(graphics::Device& device, resources::ResourceManager& resources,
                    const PostProcessSettings& settings);

        [[nodiscard]] std::string get_name() const override {
            return "Tonemap";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 216;
        }

        void render(const RenderContext& ctx) override;

      private:
        void resolve_uniforms(graphics::DeviceContext& gpu) override;

        graphics::UniformId m_tonemap_params;
        graphics::UniformId m_s_hdr;
        graphics::UniformId m_s_bloom;
        graphics::UniformId m_s_ao;
        graphics::UniformId m_s_exposure;
    };

    class FxaaPass final : public PostPass {
      public:
        FxaaPass(graphics::Device& device, resources::ResourceManager& resources,
                 const PostProcessSettings& settings);

        [[nodiscard]] std::string get_name() const override {
            return "Fxaa";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 218;
        }

        void render(const RenderContext& ctx) override;

      private:
        void resolve_uniforms(graphics::DeviceContext& gpu) override;

        graphics::UniformId m_fxaa_params;
        graphics::UniformId m_s_src;
    };
} // namespace star::rendering
