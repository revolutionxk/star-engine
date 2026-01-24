#pragma once

namespace star::graphics {
    enum class TextureUsage : u32 {
        None = 0,
        Sampled = 1 << 0,
        RenderTarget = 1 << 1,
        Storage = 1 << 2,
    };

    inline TextureUsage operator|(TextureUsage a, TextureUsage b) {
        return static_cast<TextureUsage>(static_cast<u32>(a) | static_cast<u32>(b));
    }

    inline TextureUsage operator&(TextureUsage a, TextureUsage b) {
        return static_cast<TextureUsage>(static_cast<u32>(a) & static_cast<u32>(b));
    }

    inline bool has_flag(TextureUsage flags, TextureUsage flag) {
        return (static_cast<u32>(flags) & static_cast<u32>(flag)) != 0;
    }

    enum class TextureFormat {
        RGBA8,
        RGB8,
        BGRA8,
        BGR8,
        RGBA16F,
        RGBA32F,
        D24S8,
    };

    struct TextureDescriptor {
        u32 width{0};
        u32 height{0};
        u32 mip_levels{1};
        TextureFormat format{TextureFormat::RGBA8};
        const void* initial_data{nullptr};
        size_t size_in_bytes{0};
        TextureUsage usage{TextureUsage::Sampled};
        bool generate_mips{false};
    };

    struct Texture;
} // namespace star::graphics
