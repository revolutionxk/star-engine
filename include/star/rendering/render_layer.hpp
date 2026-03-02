#pragma once
#include "star/core/types.hpp"

namespace star::rendering {
    enum class RenderLayer : u8 {
        Default = 0,
        Transparent = 64,
        Overlay = 128,
        PostProcess = 192,
    };

    constexpr u8 to_layer(RenderLayer layer) noexcept {
        return static_cast<u8>(layer);
    }
} // namespace star::rendering
