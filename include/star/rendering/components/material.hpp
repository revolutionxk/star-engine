#pragma once
#include "star/graphics/resource_handle.hpp"
#include "star/math/math.hpp"

namespace star::graphics {
    struct Shader;
    struct Texture;
} // namespace star::graphics

namespace star::components {
    struct Material {
        graphics::ResourceHandle<graphics::Shader> shader{};
        graphics::ResourceHandle<graphics::Texture> albedo_texture{};

        Vector4 albedo_color{1.0f, 1.0f, 1.0f, 1.0f};
        float metallic{0.0f};
        float roughness{0.5f};

        bool is_transparent{false};
        bool double_sided{false};
    };
} // namespace star::components
