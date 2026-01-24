#include "bgfx_framebuffer.hpp"

#include <bgfx/bgfx.h>

#include "star/core/common.hpp"

namespace star::graphics {
    BGFXFramebuffer::~BGFXFramebuffer() {
        destroy();
    }

    bool BGFXFramebuffer::create(const FramebufferDescriptor& descriptor) {
        if (descriptor.width == 0 || descriptor.height == 0) {
            STAR_LOG_ERROR(LogCategory::Graphics, "Framebuffer dimensions cannot be zero");
            return false;
        }

        if (descriptor.color_attachments.empty() && !descriptor.has_depth) {
            STAR_LOG_ERROR(LogCategory::Graphics, "Framebuffer must have at least one attachment");
            return false;
        }

        destroy();

        m_width = descriptor.width;
        m_height = descriptor.height;
        m_has_depth = descriptor.has_depth;

        std::vector<bgfx::TextureHandle> bgfx_attachments;
        bgfx_attachments.reserve(descriptor.color_attachments.size() + (descriptor.has_depth ? 1 : 0));

        for (const auto& attachment : descriptor.color_attachments) {
            if (!attachment.texture.is_valid()) {
                STAR_LOG_ERROR(LogCategory::Graphics, "Invalid color attachment texture handle");
                return false;
            }

            bgfx::TextureHandle tex_handle{};
            tex_handle.idx = static_cast<u16>(attachment.texture.id);

            if (!bgfx::isValid(tex_handle)) {
                STAR_LOG_ERROR(LogCategory::Graphics, "Invalid BGFX texture handle for color attachment");
                return false;
            }

            bgfx_attachments.push_back(tex_handle);
            m_color_attachments.push_back(attachment.texture);
        }

        if (descriptor.has_depth) {
            if (!descriptor.depth_stencil_attachment.texture.is_valid()) {
                STAR_LOG_ERROR(LogCategory::Graphics, "Invalid depth/stencil attachment texture handle");
                return false;
            }

            bgfx::TextureHandle depth_handle{};
            depth_handle.idx = static_cast<u16>(descriptor.depth_stencil_attachment.texture.id);

            if (!bgfx::isValid(depth_handle)) {
                STAR_LOG_ERROR(LogCategory::Graphics, "Invalid BGFX texture handle for depth attachment");
                return false;
            }

            bgfx_attachments.push_back(depth_handle);
            m_depth_attachment = descriptor.depth_stencil_attachment.texture;
        }

        const auto num_attachments = static_cast<u8>(bgfx_attachments.size());
        m_handle = bgfx::createFrameBuffer(num_attachments, bgfx_attachments.data(), false);

        if (!bgfx::isValid(m_handle)) {
            STAR_LOG_ERROR(LogCategory::Graphics, "Failed to create BGFX framebuffer ({}x{}, {} attachments)", m_width,
                           m_height, num_attachments);

            m_color_attachments.clear();
            m_depth_attachment = ResourceHandle<Texture>{0, 0};
            m_width = 0;
            m_height = 0;
            m_has_depth = false;

            return false;
        }

        STAR_LOG_DEBUG(LogCategory::Graphics, "Created framebuffer (handle: {}, size: {}x{}, color: {}, depth: {})",
                       m_handle.idx, m_width, m_height, m_color_attachments.size(), m_has_depth);

        return true;
    }

    void BGFXFramebuffer::destroy() {
        if (bgfx::isValid(m_handle)) {
            bgfx::destroy(m_handle);
            m_handle = BGFX_INVALID_HANDLE;
        }

        m_color_attachments.clear();
        m_depth_attachment = ResourceHandle<Texture>{0, 0};
        m_width = 0;
        m_height = 0;
        m_has_depth = false;
    }
} // namespace star::graphics
