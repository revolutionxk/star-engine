#pragma once

#include <memory>

#include "star/core/types.hpp"
#include "star/graphics/device_context.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/rendering/render_target.hpp"
#include "star/rendering/renderer.hpp"

namespace star::graphics {
    class Device;
    struct Shader;
} // namespace star::graphics

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::rendering {
    class Viewport;

    class PostProcessPass final : public IRenderPass {
      public:
        PostProcessPass(graphics::Device& device, resources::ResourceManager& resources);
        ~PostProcessPass() override;

        [[nodiscard]] std::string get_name() const override {
            return "PostProcessPass";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 210;
        }

        void render(const RenderContext& ctx) override;

        struct Settings {
            bool enabled = true;
            f32 exposure = 1.0f;
            bool bloom_enabled = true;
            f32 bloom_threshold = 1.1f;
            f32 bloom_knee = 0.5f;
            f32 bloom_intensity = 0.5f;
            bool fxaa_enabled = true;
            bool ssao_enabled = true;
            f32 ssao_radius = 0.6f;
            f32 ssao_power = 1.5f;
            f32 ssao_strength = 1.0f;
            f32 ssao_fade = 25.0f;
            bool ssr_enabled = true;
            f32 ssr_intensity = 1.0f;
            f32 ssr_max_distance = 8.0f;
            f32 ssr_thickness = 0.6f;
            bool taa_enabled = true;
            f32 taa_blend = 0.9f;
        };

        [[nodiscard]] Settings& settings() noexcept {
            return m_settings;
        }

      private:
        struct UniformIds {
            graphics::UniformId post_params;
            graphics::UniformId bloom_params;
            graphics::UniformId blur_params;
            graphics::UniformId fxaa_params;
            graphics::UniformId ssao_inv_proj;
            graphics::UniformId ssao_proj;
            graphics::UniformId ssao_params;
            graphics::UniformId ssao_texel;
            graphics::UniformId ssao_blur_params;
            graphics::UniformId ssr_proj;
            graphics::UniformId ssr_inv_proj;
            graphics::UniformId ssr_params;
            graphics::UniformId ssr_texel;
            graphics::UniformId taa_cur_inv_vp;
            graphics::UniformId taa_prev_vp;
            graphics::UniformId taa_params;
            graphics::UniformId taa_texel;

            graphics::UniformId s_src;
            graphics::UniformId s_hdr;
            graphics::UniformId s_bloom;
            graphics::UniformId s_ao;
            graphics::UniformId s_depth;
            graphics::UniformId s_gbuffer;
            graphics::UniformId s_scene;
            graphics::UniformId s_current;
            graphics::UniformId s_history;
            graphics::UniformId s_velocity;

            [[nodiscard]] bool is_resolved() const noexcept {
                return post_params.is_valid();
            }
        };

        void ensure_shaders();
        void resolve_uniforms(graphics::DeviceContext& gpu);
        static void draw_fullscreen(graphics::DeviceContext& gpu, u32 view_id,
                                    graphics::ResourceHandle<graphics::Shader> shader);

        graphics::Device& m_device;
        resources::ResourceManager& m_resources;

        graphics::ResourceHandle<graphics::Shader> m_tonemap_shader;
        graphics::ResourceHandle<graphics::Shader> m_bright_shader;
        graphics::ResourceHandle<graphics::Shader> m_blur_shader;
        graphics::ResourceHandle<graphics::Shader> m_fxaa_shader;
        graphics::ResourceHandle<graphics::Shader> m_ssao_shader;
        graphics::ResourceHandle<graphics::Shader> m_ssao_blur_shader;
        graphics::ResourceHandle<graphics::Shader> m_ssr_shader;
        graphics::ResourceHandle<graphics::Shader> m_taa_shader;

        Settings m_settings;
        UniformIds m_ids;

        u64 m_frame = ~0ull;
        u32 m_view_cursor = 0;
    };
} // namespace star::rendering
