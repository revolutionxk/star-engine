#pragma once

#include <memory>
#include <vector>

#include "star/core/types.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/graphics/texture.hpp"

namespace star::graphics {
    struct FramebufferAttachment {
        ResourceHandle<Texture> texture;
        u32 mip_level{0};
        u32 layer{0};
    };

    struct FramebufferDescriptor {
        u32 width{0};
        u32 height{0};
        std::vector<FramebufferAttachment> color_attachments;
        FramebufferAttachment depth_stencil_attachment;
        bool has_depth{false};
    };

    struct Framebuffer;

    class Device;
} // namespace star::graphics
