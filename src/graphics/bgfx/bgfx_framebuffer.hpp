#pragma once

#include <vector>

#include <bgfx/bgfx.h>

#include "star/core/types.hpp"
#include "star/graphics/framebuffer.hpp"
#include "star/graphics/resource_handle.hpp"

namespace star::graphics {
    class BGFXFramebuffer {
      public:
        BGFXFramebuffer() = default;
        ~BGFXFramebuffer();

        bool create(const FramebufferDescriptor& descriptor);

        void destroy();

        bgfx::FrameBufferHandle handle() const {
            return m_handle;
        }

        bool is_valid() const {
            return bgfx::isValid(m_handle);
        }

        Vector2 size() const {
            return {static_cast<f32>(m_width), static_cast<f32>(m_height)};
        }

        u32 width() const {
            return m_width;
        }

        u32 height() const {
            return m_height;
        }

        u32 color_attachment_count() const {
            return static_cast<u32>(m_color_attachments.size());
        }

        ResourceHandle<Texture> get_color_attachment(const u32 index) const {
            if (index < m_color_attachments.size()) {
                return m_color_attachments[index];
            }
            return ResourceHandle<Texture>{0, 0};
        }

        ResourceHandle<Texture> get_depth_attachment() const {
            return m_depth_attachment;
        }

        bool has_depth() const {
            return m_has_depth;
        }

      private:
        bgfx::FrameBufferHandle m_handle{BGFX_INVALID_HANDLE};
        std::vector<ResourceHandle<Texture>> m_color_attachments;
        ResourceHandle<Texture> m_depth_attachment{0, 0};
        u32 m_width{0};
        u32 m_height{0};
        bool m_has_depth{false};
    };
} // namespace star::graphics
