#pragma once

#include "star/core/types.hpp"

namespace star::graphics {
    enum class ClearFlags : u8 {
        None = 0,
        Color = 1 << 0,
        Depth = 1 << 1,
        Stencil = 1 << 2,
        ColorDepth = Color | Depth,
    };

    [[nodiscard]] constexpr ClearFlags operator|(const ClearFlags a, const ClearFlags b) noexcept {
        return static_cast<ClearFlags>(static_cast<u8>(a) | static_cast<u8>(b));
    }

    [[nodiscard]] constexpr bool has_flag(const ClearFlags value, const ClearFlags flag) noexcept {
        return (static_cast<u8>(value) & static_cast<u8>(flag)) != 0;
    }

    enum class BlendMode {
        Opaque,
        AlphaBlend,
        Additive,
        Premultiplied
    };
    enum class CullMode {
        None,
        Back,
        Front
    };
    enum class DepthTest {
        Less,
        LessEqual,
        Equal,
        Always,
        None
    };
    enum class PrimitiveType {
        Triangles,
        Lines,
        Points,
    };

    struct PipelineState {
        BlendMode blend_mode = BlendMode::Opaque;
        CullMode cull = CullMode::Back;
        DepthTest depth_test = DepthTest::LessEqual;
        PrimitiveType primitive = PrimitiveType::Triangles;
        bool depth_write = true;
        bool wireframe = false;
    };
} // namespace star::graphics
