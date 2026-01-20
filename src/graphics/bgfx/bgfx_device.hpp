#pragma once
#include "star/graphics/device.hpp"
#include "star/graphics/device_context.hpp"

namespace star::platform {
    class Window;
}

namespace star::graphics {
    struct BufferDescriptor;

    class BGFXDevice final : public Device {
        using Super = DeviceContext;

      public:
        BGFXDevice() = default;
        ~BGFXDevice() override;

        std::unique_ptr<DeviceContext> create_context() override;

        std::string name() const override {
            return "BGFX";
        }

        DeviceCaps caps() const override;

        ResourceHandle<Buffer> create_buffer(BufferDescriptor& buffer_descriptor) override;
        ResourceHandle<Texture> create_texture(TextureDescriptor& texture_descriptor) override;
        ResourceHandle<Shader> create_shader(ShaderDescriptor& shader_descriptor) override;

        void destroy_buffer(ResourceHandle<Buffer> handle) override;
        void destroy_texture(ResourceHandle<Texture> handle) override;
        void destroy_shader(ResourceHandle<Shader> handle) override;

        bool initialize(GraphicsDeviceConfig& device) override;
        void shutdown() override;
    };
} // namespace star::graphics
