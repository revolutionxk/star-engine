#pragma once
#include "pipeline_state.hpp"
#include "resource_handle.hpp"
#include "star/core/types.hpp"

namespace star::graphics {
    struct Buffer;
    struct Shader;
    struct Texture;
    struct Framebuffer;

    class Device;

    enum class UniformType {
        Vec4,
        Mat3,
        Mat4,
        Sampler,
    };

    enum class VertexLayoutType {
        Standard,
        Debug,
        ScreenPos,
    };

    class DeviceContext {
      public:
        explicit DeviceContext(Device* device, void* native_window_handle);
        virtual ~DeviceContext() = default;

        Device* device() const {
            return m_device;
        }

        virtual void begin_frame() = 0;
        virtual void present() = 0;
        virtual void end_frame() = 0;

        virtual void resize(u32 width, u32 height) = 0;

        virtual void touch(u32 view_id) = 0;
        virtual void set_view_clear(u32 view_id, u32 clear_flags, u32 rgba, f32 depth, u8 stencil) = 0;
        virtual void set_view_rect(u32 view_id, u16 x, u16 y, u16 width, u16 height) = 0;
        virtual void set_view_transform(u32 view_id, const Matrix4& view, const Matrix4& projection) = 0;
        virtual void set_view_framebuffer(u32 view_id, ResourceHandle<Framebuffer> handle) = 0;

        virtual void set_vertex_buffer(u8 stream, ResourceHandle<Buffer> handle) = 0;
        virtual void set_index_buffer(ResourceHandle<Buffer> handle) = 0;
        virtual void set_texture(u8 stage, ResourceHandle<Texture> handle) = 0;

        // Uploads transient (per-frame) geometry from host memory. No ResourceHandle needed —
        // BGFX owns the lifetime for the duration of the frame.
        virtual void set_transient_vertex_buffer(u8 stream, const void* data, u32 num_vertices,
                                                 VertexLayoutType layout_type) = 0;
        virtual void set_transient_index_buffer(const void* data, u32 num_indices, bool is_32bit = false) = 0;
        virtual void set_uniform(const std::string& name, const void* data, u16 num = 1,
                                 UniformType type = UniformType::Vec4) = 0;

        virtual void set_transform(const Matrix4& model) = 0;

        virtual void set_pipeline_state(const PipelineState& state) = 0;
        virtual void set_state(u64 state) = 0;

        virtual u32 submit(u32 view_id, ResourceHandle<Shader> program) = 0;

        void* native_window_handle() const {
            return m_native_window_handle;
        }

      protected:
        Device* m_device = nullptr;
        void* m_native_window_handle = nullptr;
    };
} // namespace star::graphics
