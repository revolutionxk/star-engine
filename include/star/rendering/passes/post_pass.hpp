#pragma once

#include <string_view>

#include "star/core/types.hpp"
#include "star/graphics/device_context.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/rendering/post_process_settings.hpp"
#include "star/rendering/renderer.hpp"
#include "star/resources/shader/builtin_shaders.hpp"

namespace star::graphics {
    class Device;
    struct Shader;
} // namespace star::graphics

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::rendering {
    class RenderTarget;
    class Viewport;

    namespace resource_names {
        inline constexpr std::string_view HDR = "hdr";
        inline constexpr std::string_view NORMAL = "normal";
        inline constexpr std::string_view VELOCITY = "velocity";
        inline constexpr std::string_view DEPTH = "depth";
        inline constexpr std::string_view LIT = "lit";
        inline constexpr std::string_view AO = "ao";
        inline constexpr std::string_view BLOOM = "bloom";
        inline constexpr std::string_view LDR = "ldr";
    } // namespace resource_names

    class PostPass : public IRenderPass {
      public:
        [[nodiscard]] PassScope scope() const override {
            return PassScope::PerView;
        }

      protected:
        PostPass(graphics::Device& device, resources::ResourceManager& resources,
                 const PostProcessSettings& settings, const char* shader_name, resources::BuiltinShader shader_id);

        bool ensure_ready(graphics::DeviceContext& gpu);

        virtual void resolve_uniforms(graphics::DeviceContext& gpu) = 0;

        void begin_view(graphics::DeviceContext& gpu, u32 view_id, const RenderTarget& target) const;
        void begin_view(graphics::DeviceContext& gpu, u32 view_id,
                        graphics::ResourceHandle<graphics::Framebuffer> framebuffer, u16 width, u16 height) const;

        void draw_fullscreen(graphics::DeviceContext& gpu, u32 view_id) const;
        static void draw_fullscreen(graphics::DeviceContext& gpu, u32 view_id,
                                    graphics::ResourceHandle<graphics::Shader> shader);

        [[nodiscard]] f32 flip_v() const;

        graphics::Device& m_device;
        resources::ResourceManager& m_resources;
        const PostProcessSettings& m_settings;

        const char* m_shader_name;
        resources::BuiltinShader m_shader_id;
        graphics::ResourceHandle<graphics::Shader> m_shader;

        graphics::UniformId m_post_params;
        bool m_uniforms_resolved = false;
    };
} // namespace star::rendering
