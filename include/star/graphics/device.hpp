#pragma once
#include "resource_handle.hpp"
#include "star/platform/platform_data.hpp"

namespace star::graphics {
    struct BufferDescriptor;
    struct ShaderDescriptor;
    struct TextureDescriptor;

    struct Buffer;
    struct Shader;
    struct Texture;

    class DeviceContext;

    enum class GraphicsAPI {
        Auto,
        BGFX,
    };

    struct GraphicsDeviceConfig {
        GraphicsAPI api = GraphicsAPI::Auto;
        platform::PlatformData platform{};
        Vector2 window_size{};
    };

    struct DeviceCaps {
        std::string renderer_name{};
        std::string vendor_name{};
        std::string version_string{};

        int max_texture_size = 0;
        int max_texture_units = 0;
        bool supports_compute_shaders = false;
    };

    class Device {
      public:
        virtual ~Device() = default;

        virtual bool initialize(GraphicsDeviceConfig& device) = 0;
        virtual void shutdown() = 0;
        virtual std::string name() const = 0;
        virtual DeviceCaps caps() const = 0;

        virtual std::unique_ptr<DeviceContext> create_context() = 0;

        virtual ResourceHandle<Buffer> create_buffer(BufferDescriptor& buffer_descriptor) = 0;
        virtual ResourceHandle<Texture> create_texture(TextureDescriptor& buffer_descriptor) = 0;
        virtual ResourceHandle<Shader> create_shader(ShaderDescriptor& buffer_descriptor) = 0;

        virtual void destroy_buffer(ResourceHandle<Buffer> handle) = 0;
        virtual void destroy_texture(ResourceHandle<Texture> handle) = 0;
        virtual void destroy_shader(ResourceHandle<Shader> handle) = 0;

        bool is_initialized() const {
            return m_initialized;
        }

        static std::unique_ptr<Device> create(GraphicsAPI api = GraphicsAPI::Auto);

      protected:
        bool m_initialized = false;
        GraphicsDeviceConfig m_config;
    };
} // namespace star::graphics
