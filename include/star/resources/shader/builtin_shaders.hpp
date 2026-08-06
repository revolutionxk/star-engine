#pragma once

#include "star/core/types.hpp"

namespace star::resources {
    enum class BuiltinShader : u8 {
        Simple,
        Material,
        ImGui,
        Atmosphere,
        Debug,
        Picking,
        Shadow,
        Tonemap,
        BloomBright,
        BloomBlur,
        BloomDown,
        BloomUp,
        Fxaa,
        Ssao,
        SsaoBlur,
        Ssr,
        Taa,
    };
} // namespace star::resources
