#pragma once

namespace star::graphics {
    struct TextureDescriptor {
        enum class Format {
            RGBA8,
            RGB8,
            BGRA8,
            BGR8,
            RGBA16F,
            RGBA32F,
            Depth24Stencil8,
        };

        u32 width{0};
        u32 height{0};
        u32 mip_levels{1};
        Format format{Format::RGBA8};
        const void* initial_data{nullptr};
        size_t size_in_bytes{0};
    };

    struct Texture;
} // namespace star::graphics
