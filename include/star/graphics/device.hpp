#pragma once
#include "resource_handle.hpp"
#include "star/platform/platform_data.hpp"

namespace star::graphics {
    struct BufferDescriptor;
    struct ShaderDescriptor;
    struct TextureDescriptor;
    struct FramebufferDescriptor;

    struct Buffer;
    struct Shader;
    struct Texture;
    struct Framebuffer;

    class DeviceContext;

    enum class GraphicsAPI {
        Auto,
        BGFX,
    };

    struct GraphicsDeviceConfig {
        GraphicsAPI api = GraphicsAPI::Auto;
        platform::PlatformData platform{};
        Vector2 window_size{};
        bool vsync = true;
        bool debug = false;
        bool profile = false;
    };

    struct DeviceCaps {
        std::string renderer_name{};
        std::string vendor_name{};
        std::string version_string{};

        int max_texture_size = 0;
        int max_texture_units = 0;
        bool supports_compute_shaders = false;
        bool origin_bottom_left = false;
        bool homogeneous_depth = false;
    };

    class Device {
      public:
        virtual ~Device() = default;

        virtual std::string name() const = 0;
        virtual DeviceCaps caps() const = 0;

        virtual ResourceHandle<Buffer> create_buffer(BufferDescriptor& buffer_descriptor) = 0;
        virtual ResourceHandle<Texture> create_texture(TextureDescriptor& buffer_descriptor) = 0;
        
        virtual ResourceHandle<Texture> create_readback_texture(u16 width, u16 height) = 0;
        virtual ResourceHandle<Shader> create_shader(ShaderDescriptor& buffer_descriptor) = 0;
        virtual ResourceHandle<Framebuffer> create_framebuffer(FramebufferDescriptor& framebuffer_descriptor) = 0;

        virtual void destroy_buffer(ResourceHandle<Buffer> handle) = 0;
        virtual void destroy_texture(ResourceHandle<Texture> handle) = 0;
        virtual void destroy_shader(ResourceHandle<Shader> handle) = 0;
        virtual void destroy_framebuffer(ResourceHandle<Framebuffer> handle) = 0;

        virtual ResourceHandle<Shader> reload_shader(const ResourceHandle<Shader> old_handle,
                                                     ShaderDescriptor& descriptor) {
            const auto new_handle = create_shader(descriptor);
            if (new_handle.is_valid() && old_handle.is_valid()) {
                destroy_shader(old_handle);
            }
            return new_handle;
        }

        bool is_initialized() const {
            return m_initialized;
        }

        DeviceContext* context() const {
            return m_context.get();
        }

        std::shared_ptr<DeviceContext> create_context() {
            return m_context;
        }

        static std::shared_ptr<Device> create(const GraphicsDeviceConfig& config);

      protected:
        bool m_initialized = false;
        GraphicsDeviceConfig m_config;
        std::shared_ptr<DeviceContext> m_context;
    };
} // namespace star::graphics
