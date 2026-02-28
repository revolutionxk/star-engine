#pragma once
#include <string>
#include <variant>

#include "star/graphics/resource_handle.hpp"
#include "star/math/math.hpp"

namespace star::graphics {
    struct Texture;
} // namespace star::graphics

namespace star::rendering {
    struct MaterialProperty {
        using Value =
            std::variant<float, Vector2, Vector3, Vector4, Matrix4, graphics::ResourceHandle<graphics::Texture>>;

        std::string name;
        Value value;

        u8 texture_stage{0};
    };
} // namespace star::rendering
