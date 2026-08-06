#pragma once

#include "star/math/math.hpp"

namespace star::graphics {
    struct Vertex {
        Vector3 position;
        Vector3 normal;
        Vector2 tex_coords;
        Vector3 tangent;
        Vector3 bitangent;
    };
} // namespace star::graphics
