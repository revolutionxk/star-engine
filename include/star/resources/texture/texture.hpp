#pragma once
#include "star/resources/resource.hpp"
#include "star/resources/resource_manager.hpp"

namespace star::resources {
    using namespace star::graphics;

    enum class TextureFormat {
        R8,
        RG8,
        RGB8,
        RGBA8,
        RGBA16F,
        RGBA32F,
        Depth24Stencil8
    };
    enum class TextureFilter {
        Nearest,
        Linear,
        Trilinear
    };

    enum class TextureWrap {
        Repeat,
        Clamp,
        Mirror
    };

    struct TextureDescriptor {
        u32 width = 0;
        u32 height = 0;
        TextureFormat format = TextureFormat::RGBA8;
        TextureFilter filter = TextureFilter::Linear;
        TextureWrap wrap = TextureWrap::Repeat;
        bool generate_mipmaps = true;
    };

    struct Texture : Resource {
        std::vector<u8> data{};
        ResourceHandle<Texture> handle{};
        TextureDescriptor desc{};

        u32 mipmap_count() const {
            if (!desc.generate_mipmaps) {
                return 1;
            }
            return static_cast<u32>(std::floor(std::log2(std::max(desc.width, desc.height)))) + 1;
        }

        u32 width() const {
            return desc.width;
        }

        u32 height() const {
            return desc.height;
        }
    };
} // namespace star::resources
