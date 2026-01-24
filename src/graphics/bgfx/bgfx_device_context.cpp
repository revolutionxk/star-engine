#include "bgfx_device_context.hpp"

#include "bgfx/bgfx.h"
#include "star/core/common.hpp"

namespace star::graphics {
    BGFXDeviceContext::BGFXDeviceContext(Device* device, void* native_window_handle)
        : DeviceContext(device, native_window_handle) {}

    BGFXDeviceContext::~BGFXDeviceContext() {}

    void BGFXDeviceContext::begin_frame() {
        bgfx::touch(m_current_view);
    }

    void BGFXDeviceContext::present() {}

    void BGFXDeviceContext::end_frame() {
        bgfx::frame();
    }

    void BGFXDeviceContext::resize(const u32 width, const u32 height) {
        if (width == 0 || height == 0) {
            STAR_LOG_WARN(LogCategory::Graphics, "Invalid resize dimensions: {}x{}", width, height);
            return;
        }

        constexpr u32 reset_flags = BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X4;

        bgfx::reset(width, height, reset_flags);
        bgfx::setViewRect(m_current_view, 0, 0, width, height);

        STAR_LOG_DEBUG(LogCategory::Graphics, "Device context resized to {}x{}", width, height);
    }

    void BGFXDeviceContext::set_view_clear(const u32 view_id, const u32 clear_flags, const u32 rgba, const f32 depth,
                                           const u8 stencil) {
        m_current_view = view_id;

        u16 bgfx_flags = 0;
        if (clear_flags & 0x1)
            bgfx_flags |= BGFX_CLEAR_COLOR;
        if (clear_flags & 0x2)
            bgfx_flags |= BGFX_CLEAR_DEPTH;
        if (clear_flags & 0x4)
            bgfx_flags |= BGFX_CLEAR_STENCIL;

        bgfx::setViewClear(static_cast<bgfx::ViewId>(view_id), bgfx_flags, rgba, depth, stencil);
    }

    void BGFXDeviceContext::set_view_rect(const u32 view_id, const u16 x, const u16 y, const u16 width,
                                          const u16 height) {
        bgfx::setViewRect(static_cast<bgfx::ViewId>(view_id), x, y, width, height);
    }

    void BGFXDeviceContext::set_view_transform(const u32 view_id, const Matrix4& view, const Matrix4& projection) {
        bgfx::setViewTransform(static_cast<bgfx::ViewId>(view_id), view.data(), projection.data());
    }

    void BGFXDeviceContext::set_view_framebuffer(const u32 view_id, const ResourceHandle<Framebuffer> handle) {
        if (!handle.is_valid()) {
            bgfx::setViewFrameBuffer(static_cast<bgfx::ViewId>(view_id), BGFX_INVALID_HANDLE);
            return;
        }

        if (const bgfx::FrameBufferHandle fb{static_cast<u16>(handle.id)}; bgfx::isValid(fb)) {
            bgfx::setViewFrameBuffer(static_cast<bgfx::ViewId>(view_id), fb);
        } else {
            STAR_LOG_WARN(LogCategory::Graphics, "Invalid framebuffer handle for view {}", view_id);
        }
    }

    void BGFXDeviceContext::set_vertex_buffer(const u8 stream, const ResourceHandle<Buffer> handle) {
        if (!handle.is_valid()) {
            STAR_LOG_WARN(LogCategory::Graphics, "Attempted to set invalid vertex buffer");
            return;
        }

        if (const bgfx::VertexBufferHandle vb{static_cast<u16>(handle.id)}; bgfx::isValid(vb)) {
            bgfx::setVertexBuffer(stream, vb);
        } else {
            // Try dynamic vertex buffer
            if (const bgfx::DynamicVertexBufferHandle dvb{static_cast<u16>(handle.id)}; bgfx::isValid(dvb)) {
                bgfx::setVertexBuffer(stream, dvb);
            }
        }
    }

    void BGFXDeviceContext::set_index_buffer(ResourceHandle<Buffer> handle) {
        if (!handle.is_valid()) {
            STAR_LOG_WARN(LogCategory::Graphics, "Attempted to set invalid index buffer");
            return;
        }

        if (const bgfx::IndexBufferHandle ib{static_cast<u16>(handle.id)}; bgfx::isValid(ib)) {
            bgfx::setIndexBuffer(ib);
        } else {
            if (const bgfx::DynamicIndexBufferHandle dib{static_cast<u16>(handle.id)}; bgfx::isValid(dib)) {
                bgfx::setIndexBuffer(dib);
            }
        }
    }

    void BGFXDeviceContext::set_texture(const u8 stage, const ResourceHandle<Texture> handle) {
        if (!handle.is_valid()) {
            return;
        }

        if (const bgfx::TextureHandle texture{static_cast<u16>(handle.id)}; bgfx::isValid(texture)) {
            // I haven't finished it yet, but I intend to make this device the basis for other things
            const bgfx::UniformHandle sampler = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
            bgfx::setTexture(stage, sampler, texture);
        }
    }

    void BGFXDeviceContext::set_transform(const Matrix4& model) {
        bgfx::setTransform(model.data());
    }

    u32 BGFXDeviceContext::submit(const u32 view_id, const ResourceHandle<Shader> program) {
        if (!program.is_valid()) {
            STAR_LOG_WARN(LogCategory::Graphics, "Attempted to submit with invalid shader program");
            return 0;
        }

        bgfx::ProgramHandle prog{static_cast<u16>(program.id)};
        if (!bgfx::isValid(prog)) {
            STAR_LOG_WARN(LogCategory::Graphics, "Invalid BGFX program handle");
            return 0;
        }
        constexpr auto state = 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
                               BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA;

        bgfx::setState(state);
        bgfx::submit(static_cast<bgfx::ViewId>(view_id), prog);

        return 1;
    }
} // namespace star::graphics
