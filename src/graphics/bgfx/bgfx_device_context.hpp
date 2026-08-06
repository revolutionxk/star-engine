#pragma once
#include <unordered_map>

#include "bgfx/bgfx.h"
#include "star/graphics/device_context.hpp"

namespace star::graphics {
    struct TransparentStringHash {
        using is_transparent = void;

        [[nodiscard]] std::size_t operator()(const std::string_view name) const noexcept {
            return std::hash<std::string_view>{}(name);
        }
    };

    class BGFXDeviceContext : public DeviceContext {
      public:
        BGFXDeviceContext(Device* device, void* native_window_handle, u32 reset_flags);
        ~BGFXDeviceContext() override;

        void begin_frame() override;
        void present() override;
        void end_frame() override;

        void resize(u32 width, u32 height) override;

        void touch(u32 view_id) override;
        void set_view_name(u32 view_id, std::string_view name) override;
        void set_view_clear(u32 view_id, ClearFlags flags, u32 rgba, f32 depth, u8 stencil) override;
        void set_view_rect(u32 view_id, u16 x, u16 y, u16 width, u16 height) override;
        void set_view_transform(u32 view_id, const Matrix4& view, const Matrix4& projection) override;
        void set_view_framebuffer(u32 view_id, ResourceHandle<Framebuffer> handle) override;

        void set_vertex_buffer(u8 stream, ResourceHandle<Buffer> handle) override;
        void set_index_buffer(ResourceHandle<Buffer> handle) override;
        void set_texture(UniformId sampler, u8 stage, ResourceHandle<Texture> handle) override;
        void set_transient_vertex_buffer(u8 stream, const void* data, u32 num_vertices,
                                         VertexLayoutType layout_type) override;
        void set_transient_index_buffer(const void* data, u32 num_indices, bool is_32bit = false) override;

        UniformId uniform(std::string_view name, UniformType type, u16 num = 1) override;
        void set_uniform(UniformId id, const void* data, u16 num = 1) override;

        void set_transform(const Matrix4& model) override;
        void set_pipeline_state(const PipelineState& state) override;
        void set_state(u64 state) override;

        u32 submit(u32 view_id, ResourceHandle<Shader> program) override;

        void blit(u32 view_id, ResourceHandle<Texture> dst, u16 dst_x, u16 dst_y, ResourceHandle<Texture> src,
                  u16 src_x, u16 src_y, u16 width, u16 height) override;
        u32 read_texture(ResourceHandle<Texture> texture, void* data) override;
        [[nodiscard]] u32 current_frame() const override;

        [[nodiscard]] FrameStats frame_stats() const override;
        void collect_view_stats(std::vector<ViewStats>& out) const override;

      private:
        u32 submit_internal(u32 view_id, ResourceHandle<Shader> program, u8 discard_flags);

        u32 m_current_view{0};
        u64 m_next_state{0};
        u32 m_frame_number{0};
        u32 m_reset_flags{0};
        std::unordered_map<std::string, bgfx::UniformHandle, TransparentStringHash, std::equal_to<>> m_uniform_cache;
    };
} // namespace star::graphics
