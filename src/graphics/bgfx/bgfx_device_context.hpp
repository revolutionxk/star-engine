#pragma once
#include "star/graphics/device_context.hpp"

namespace star::graphics {
    class BGFXDeviceContext : public DeviceContext {
      public:
        explicit BGFXDeviceContext(void* native_window_handle);
        ~BGFXDeviceContext() override;

        void begin_frame() override;
        void present() override;
        void end_frame() override;

        void resize(u32 width, u32 height) override;

        void set_view_clear(u32 view_id, u32 clear_flags, u32 rgba, f32 depth, u8 stencil) override;
        void set_view_rect(u32 view_id, u16 x, u16 y, u16 width, u16 height) override;
        void set_view_transform(u32 view_id, const Matrix4& view, const Matrix4& projection) override;
        
        void set_vertex_buffer(u8 stream, ResourceHandle<Buffer> handle) override;
        void set_index_buffer(ResourceHandle<Buffer> handle) override;
        void set_texture(u8 stage, ResourceHandle<Texture> handle) override;
        
        void set_transform(const Matrix4& model) override;
        
        u32 submit(u32 view_id, ResourceHandle<Shader> program) override;

      private:
        u32 m_current_view{0};
    };
} // namespace star::graphics
