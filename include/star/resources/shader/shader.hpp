#pragma once
#include "star/graphics/resource_handle.hpp"
#include "star/graphics/shader.hpp"
#include "star/resources/resource.hpp"

namespace star::resources {
    using namespace star::graphics;

    struct Shader : Resource {
        ResourceHandle<graphics::Shader> handle{};

        std::string disk_vertex_path;
        std::string disk_fragment_path;

        bool is_valid() const {
            return handle.is_valid();
        }
    };
} // namespace star::resources
