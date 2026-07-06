#pragma once

#include <memory>
#include <unordered_map>

#include "star/core/types.hpp"
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
        };

        [[nodiscard]] Settings& settings() noexcept {
            return m_settings;
        }

      private:
        struct BloomTargets {
            std::unique_ptr<RenderTarget> a;
            std::unique_ptr<RenderTarget> b;
            u32 width = 0;
            u32 height = 0;
        };

        struct ResolveTarget {
            std::unique_ptr<RenderTarget> ldr;
            u32 width = 0;
            u32 height = 0;
        };

        void ensure_shaders();
        static void draw_fullscreen(graphics::DeviceContext& gpu, u32 view_id,
                                    graphics::ResourceHandle<graphics::Shader> shader);

        BloomTargets* bloom_targets_for(const Viewport& viewport);
        ResolveTarget* resolve_target_for(const Viewport& viewport);

        graphics::Device& m_device;
        resources::ResourceManager& m_resources;

        graphics::ResourceHandle<graphics::Shader> m_tonemap_shader;
        graphics::ResourceHandle<graphics::Shader> m_bright_shader;
        graphics::ResourceHandle<graphics::Shader> m_blur_shader;
        graphics::ResourceHandle<graphics::Shader> m_fxaa_shader;

        Settings m_settings;
        std::unordered_map<const Viewport*, BloomTargets> m_bloom;
        std::unordered_map<const Viewport*, ResolveTarget> m_resolve;

        u64 m_frame = ~0ull;
        u32 m_view_cursor = 0;
    };
} // namespace star::rendering
