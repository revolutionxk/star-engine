#pragma once

#include <memory>

#include "star/core/types.hpp"
#include "star/graphics/texture.hpp"

namespace star::graphics {
    class Framebuffer {
      public:
        virtual ~Framebuffer() = default;

        virtual void bind() = 0;
        virtual void unbind() = 0;

        virtual void attach_color_texture(const std::shared_ptr<Texture>& texture, u32 attachment_index) = 0;
        virtual void attach_depth_texture(const std::shared_ptr<Texture>& texture) = 0;

        virtual bool is_complete() const = 0;

        virtual Vector2 size() const = 0;
    };
} // namespace star::graphics
