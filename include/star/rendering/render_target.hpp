#pragma once

#include <memory>

#include "star/core/types.hpp"
#include "star/graphics/device.hpp"
#include "star/graphics/framebuffer.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/graphics/texture.hpp"

namespace star::rendering {
    class RenderTarget {
      public:
        RenderTarget() = default;
        ~RenderTarget();

        bool create(graphics::Device* device, u32 width, u32 height,
                    graphics::TextureDescriptor::Format color_format = graphics::TextureDescriptor::Format::RGBA8,
                    bool has_depth = true);

        bool create_mrt(graphics::Device* device, u32 width, u32 height,
                        const graphics::TextureDescriptor::Format* color_formats, u32 num_color_attachments,
                        bool has_depth = true);

        void destroy();

        bool resize(u32 width, u32 height);

        [[nodiscard]] graphics::ResourceHandle<graphics::Framebuffer> framebuffer() const {
            return m_framebuffer;
        }

        [[nodiscard]] graphics::ResourceHandle<graphics::Texture> color_texture(const u32 index = 0) const {
            if (index < m_color_textures.size()) {
                return m_color_textures[index];
            }
            return graphics::ResourceHandle<graphics::Texture>{0, 0};
        }

        [[nodiscard]] graphics::ResourceHandle<graphics::Texture> depth_texture() const {
            return m_depth_texture;
        }

        [[nodiscard]] u32 color_attachment_count() const {
            return static_cast<u32>(m_color_textures.size());
        }

        [[nodiscard]] u32 width() const {
            return m_width;
        }

        [[nodiscard]] u32 height() const {
            return m_height;
        }

        [[nodiscard]] bool is_valid() const {
            return m_framebuffer.is_valid() && !m_color_textures.empty();
        }

      private:
        graphics::Device* m_device{nullptr};
        graphics::ResourceHandle<graphics::Framebuffer> m_framebuffer{0, 0};
        std::vector<graphics::ResourceHandle<graphics::Texture>> m_color_textures;
        graphics::ResourceHandle<graphics::Texture> m_depth_texture{0, 0};

        std::vector<graphics::TextureDescriptor::Format> m_color_formats;
        bool m_has_depth{false};
        u32 m_width{0};
        u32 m_height{0};
    };
} // namespace star::rendering
