#pragma once
#include <vector>

#include "star/rendering/material_property.hpp"

namespace star::components {
    struct MaterialInstance {
        std::vector<rendering::MaterialProperty> overrides;
    };
} // namespace star::components
