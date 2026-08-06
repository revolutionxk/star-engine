#pragma once
#include <string>
#include <string_view>
#include <vector>

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

    struct UniformId {
        static constexpr u16 INVALID = 0xffff;

        u16 index{INVALID};

        [[nodiscard]] bool is_valid() const noexcept {
            return index != INVALID;
        }
    };

    struct FrameStats {
        u32 draw_calls{0};
        u32 compute_calls{0};
        u32 blit_calls{0};
        u64 triangles{0};

        f64 cpu_frame_ms{0.0};
        f64 gpu_frame_ms{0.0};
        f64 wait_submit_ms{0.0};
        f64 wait_render_ms{0.0};

        u32 textures{0};
        u32 framebuffers{0};
        u32 programs{0};
        u32 uniform_count{0};

        u64 texture_memory{0};
        u64 render_target_memory{0};
        u64 transient_vb_used{0};
        u64 transient_ib_used{0};
    };

    struct ViewStats {
        char name[16]{};
        u16 view_id{0};
        f64 cpu_ms{0.0};
        f64 gpu_ms{0.0};
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
        virtual void set_view_name(u32 view_id, std::string_view name) = 0;
        virtual void set_view_clear(u32 view_id, u32 clear_flags, u32 rgba, f32 depth, u8 stencil) = 0;
        virtual void set_view_rect(u32 view_id, u16 x, u16 y, u16 width, u16 height) = 0;
        virtual void set_view_transform(u32 view_id, const Matrix4& view, const Matrix4& projection) = 0;
        virtual void set_view_framebuffer(u32 view_id, ResourceHandle<Framebuffer> handle) = 0;

        virtual void set_vertex_buffer(u8 stream, ResourceHandle<Buffer> handle) = 0;
        virtual void set_index_buffer(ResourceHandle<Buffer> handle) = 0;
        virtual void set_texture(UniformId sampler, u8 stage, ResourceHandle<Texture> handle) = 0;

        // Uploads transient (per-frame) geometry from host memory. No ResourceHandle needed —
        // BGFX owns the lifetime for the duration of the frame.
        virtual void set_transient_vertex_buffer(u8 stream, const void* data, u32 num_vertices,
                                                 VertexLayoutType layout_type) = 0;
        virtual void set_transient_index_buffer(const void* data, u32 num_indices, bool is_32bit = false) = 0;

        virtual UniformId uniform(std::string_view name, UniformType type, u16 num = 1) = 0;
        virtual void set_uniform(UniformId id, const void* data, u16 num = 1) = 0;

        virtual void set_transform(const Matrix4& model) = 0;

        virtual void set_pipeline_state(const PipelineState& state) = 0;
        virtual void set_state(u64 state) = 0;

        virtual u32 submit(u32 view_id, ResourceHandle<Shader> program) = 0;


        virtual void blit(u32 view_id, ResourceHandle<Texture> dst, u16 dst_x, u16 dst_y, ResourceHandle<Texture> src,
                          u16 src_x, u16 src_y, u16 width, u16 height) = 0;
        virtual u32 read_texture(ResourceHandle<Texture> texture, void* data) = 0;
        [[nodiscard]] virtual u32 current_frame() const = 0;

        [[nodiscard]] virtual FrameStats frame_stats() const = 0;
        virtual void collect_view_stats(std::vector<ViewStats>& out) const = 0;

        void* native_window_handle() const {
            return m_native_window_handle;
        }

      protected:
        Device* m_device = nullptr;
        void* m_native_window_handle = nullptr;
    };
} // namespace star::graphics
