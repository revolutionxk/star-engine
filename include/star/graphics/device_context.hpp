#pragma once
#include "star/core/types.hpp"
#include "resource_handle.hpp"

namespace star::platform {
    class Window;
}

namespace star::graphics {
    struct Buffer;
    struct Shader;
    struct Texture;

    class DeviceContext {
      public:
        explicit DeviceContext(void* native_window_handle);
        virtual ~DeviceContext() = default;

        virtual void begin_frame() = 0;
        virtual void present() = 0;
        virtual void end_frame() = 0;

        virtual void resize(u32 width, u32 height) = 0;

        virtual void set_view_clear(u32 view_id, u32 clear_flags, u32 rgba, f32 depth, u8 stencil) = 0;
        virtual void set_view_rect(u32 view_id, u16 x, u16 y, u16 width, u16 height) = 0;
        virtual void set_view_transform(u32 view_id, const Matrix4& view, const Matrix4& projection) = 0;
        
        virtual void set_vertex_buffer(u8 stream, ResourceHandle<Buffer> handle) = 0;
        virtual void set_index_buffer(ResourceHandle<Buffer> handle) = 0;
        virtual void set_texture(u8 stage, ResourceHandle<Texture> handle) = 0;
        
        virtual void set_transform(const Matrix4& model) = 0;
        
        virtual u32 submit(u32 view_id, ResourceHandle<Shader> program) = 0;

        void* native_window_handle() const {
            return m_native_window_handle;
        }

      protected:
        void* m_native_window_handle = nullptr;
    };
} // namespace star::graphics
