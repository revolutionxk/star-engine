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
        explicit BGFXDevice(const GraphicsDeviceConfig& config);
        ~BGFXDevice() override;

        std::string name() const override {
            return "BGFX";
        }

        DeviceCaps caps() const override;

        ResourceHandle<Buffer> create_buffer(BufferDescriptor& buffer_descriptor) override;
        ResourceHandle<Texture> create_texture(TextureDescriptor& texture_descriptor) override;
        ResourceHandle<Shader> create_shader(ShaderDescriptor& shader_descriptor) override;
        ResourceHandle<Framebuffer> create_framebuffer(FramebufferDescriptor& framebuffer_descriptor) override;

        void destroy_buffer(ResourceHandle<Buffer> handle) override;
        void destroy_texture(ResourceHandle<Texture> handle) override;
        void destroy_shader(ResourceHandle<Shader> handle) override;
        void destroy_framebuffer(ResourceHandle<Framebuffer> handle) override;

      private:
        enum class BufferKind : u32 {
            StaticVertex,
            DynamicVertex,
            StaticIndex,
            DynamicIndex,
        };

        static u32 encode_buffer(BufferKind kind, u16 index);
        static BufferKind buffer_kind(u32 id);
    };
} // namespace star::graphics
