#include "star/rendering/render_target.hpp"

#include "star/core/common.hpp"

namespace star::rendering {
    RenderTarget::~RenderTarget() {
        destroy();
    }

    bool RenderTarget::create(graphics::Device* device, const u32 width, const u32 height,
                              const graphics::TextureDescriptor::Format color_format, const bool has_depth) {
        return create_mrt(device, width, height, &color_format, 1, has_depth);
    }

    bool RenderTarget::create_mrt(graphics::Device* device, const u32 width, const u32 height,
                                  const graphics::TextureDescriptor::Format* color_formats,
                                  const u32 num_color_attachments, const bool has_depth) {
        if (!device) {
            STAR_LOG_ERROR(LogCategory::Graphics, "RenderTarget: Device is null");
            return false;
        }

        if (width == 0 || height == 0) {
            STAR_LOG_ERROR(LogCategory::Graphics, "RenderTarget: Invalid dimensions {}x{}", width, height);
            return false;
        }

        if (num_color_attachments == 0) {
            STAR_LOG_ERROR(LogCategory::Graphics, "RenderTarget: Must have at least one color attachment");
            return false;
        }

        destroy();

        m_device = device;
        m_width = width;
        m_height = height;
        m_has_depth = has_depth;

        m_color_formats.clear();
        for (u32 i = 0; i < num_color_attachments; ++i) {
            m_color_formats.push_back(color_formats[i]);
        }

        std::vector<graphics::FramebufferAttachment> color_attachments;
        for (u32 i = 0; i < num_color_attachments; ++i) {
            graphics::TextureDescriptor color_desc{.width = width,
                                                   .height = height,
                                                   .mip_levels = 1,
                                                   .format = color_formats[i],
                                                   .initial_data = nullptr,
                                                   .size_in_bytes = 0};

            auto color_texture = m_device->create_texture(color_desc);
            if (!color_texture.is_valid()) {
                STAR_LOG_ERROR(LogCategory::Graphics, "RenderTarget: Failed to create color texture {}", i);
                destroy();
                return false;
            }

            m_color_textures.push_back(color_texture);
            color_attachments.push_back(
                graphics::FramebufferAttachment{.texture = color_texture, .mip_level = 0, .layer = 0});
        }

        graphics::FramebufferAttachment depth_attachment{};
        if (has_depth) {
            graphics::TextureDescriptor depth_desc{.width = width,
                                                   .height = height,
                                                   .mip_levels = 1,
                                                   .format = graphics::TextureDescriptor::Format::Depth24Stencil8,
                                                   .initial_data = nullptr,
                                                   .size_in_bytes = 0};

            m_depth_texture = m_device->create_texture(depth_desc);
            if (!m_depth_texture.is_valid()) {
                STAR_LOG_ERROR(LogCategory::Graphics, "RenderTarget: Failed to create depth texture");
                destroy();
                return false;
            }

            depth_attachment = graphics::FramebufferAttachment{.texture = m_depth_texture, .mip_level = 0, .layer = 0};
        }

        graphics::FramebufferDescriptor fb_desc{.width = width,
                                                .height = height,
                                                .color_attachments = color_attachments,
                                                .depth_stencil_attachment = depth_attachment,
                                                .has_depth = has_depth};

        m_framebuffer = m_device->create_framebuffer(fb_desc);

        STAR_LOG_INFO(LogCategory::Graphics, "RenderTarget created: {}x{}, {} color attachments, depth: {}", width,
                      height, num_color_attachments, has_depth);

        return true;
    }

    void RenderTarget::destroy() {
        if (!m_device) {
            return;
        }

        if (m_framebuffer.is_valid()) {
            m_device->destroy_framebuffer(m_framebuffer);
            m_framebuffer = graphics::ResourceHandle<graphics::Framebuffer>{0, 0};
        }

        for (auto& texture : m_color_textures) {
            if (texture.is_valid()) {
                m_device->destroy_texture(texture);
            }
        }
        m_color_textures.clear();

        if (m_depth_texture.is_valid()) {
            m_device->destroy_texture(m_depth_texture);
            m_depth_texture = graphics::ResourceHandle<graphics::Texture>{0, 0};
        }

        m_device = nullptr;
        m_width = 0;
        m_height = 0;
        m_has_depth = false;
        m_color_formats.clear();
    }

    bool RenderTarget::resize(const u32 width, const u32 height) {
        if (!m_device) {
            STAR_LOG_ERROR(LogCategory::Graphics, "RenderTarget: Cannot resize without device");
            return false;
        }

        if (width == m_width && height == m_height) {
            return true;
        }

        const auto num_attachments = static_cast<u32>(m_color_formats.size());
        const bool has_depth = m_has_depth;

        return create_mrt(m_device, width, height, m_color_formats.data(), num_attachments, has_depth);
    }
} // namespace star::rendering
