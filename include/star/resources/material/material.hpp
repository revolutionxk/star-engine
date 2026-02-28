#pragma once

#include "star/graphics/pipeline_state.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/math/math.hpp"
#include "star/resources/resource.hpp"

namespace star::graphics {
    struct Shader;
    struct Texture;
} // namespace star::graphics

namespace star::resources {
    struct Material : Resource {
        graphics::ResourceHandle<graphics::Shader> shader{};
        graphics::ResourceHandle<graphics::Texture> albedo_texture{};

        Vector4 albedo_color{1.0f, 1.0f, 1.0f, 1.0f};
        float metallic{0.0f};
        float roughness{0.5f};

        graphics::PipelineState pipeline_state{};

        bool is_valid() const {
            return shader.is_valid();
        }
    };
} // namespace star::resources
