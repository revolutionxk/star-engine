#pragma once

#include "star/core/types.hpp"

namespace star::resources {
    enum class BuiltinShader : u8 {
        Simple,
        Material,
        ImGui,
        Atmosphere,
        Debug,
    };
} // namespace star::resources
