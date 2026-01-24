#pragma once
#include <bgfx/bgfx.h>

#include "star/graphics/vertex.hpp"
#include "star/math/math.hpp"

namespace star::graphics {
    inline bgfx::VertexLayout create_standard_vertex_layout() {
        bgfx::VertexLayout layout;
        layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Float)
            .end();
        return layout;
    }

    static_assert(sizeof(Vertex) == sizeof(Vector3) * 4 + sizeof(Vector2), "Vertex struct size doesn't match layout");
} // namespace star::graphics
