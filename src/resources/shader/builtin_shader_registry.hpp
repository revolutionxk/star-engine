#pragma once

#include <string_view>

#include <bgfx/embedded_shader.h>

#include "star/graphics/resource_handle.hpp"
#include "star/graphics/shader.hpp"
#include "star/resources/shader/builtin_shaders.hpp"

namespace star::resources::detail {
    struct EmbeddedShaderPair {
        const bgfx::EmbeddedShader* vertex;
        const bgfx::EmbeddedShader* fragment;
    };

    [[nodiscard]] EmbeddedShaderPair embedded_pair_for(BuiltinShader id) noexcept;

    [[nodiscard]] graphics::ResourceHandle<graphics::Shader> create_embedded_program(BuiltinShader id,
                                                                                     std::string_view name);
} // namespace star::resources::detail
