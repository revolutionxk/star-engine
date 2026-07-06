#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "star/core/types.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/math/math.hpp"

namespace star::resources {
    class ResourceManager;
    struct Mesh;
    struct Material;

    struct ImportedPrimitive {
        graphics::ResourceHandle<Mesh> mesh;
        graphics::ResourceHandle<Material> material;
    };

    struct ImportedNode {
        std::string name;
        Vector3 position{0.0f, 0.0f, 0.0f};
        Quaternion rotation{0.0f, 0.0f, 0.0f, 1.0f};
        Vector3 scale{1.0f, 1.0f, 1.0f};
        std::vector<ImportedPrimitive> primitives;
        std::vector<u32> children;
    };

    struct ImportedModel {
        std::string name;
        std::vector<ImportedNode> nodes;
        std::vector<u32> roots;

        [[nodiscard]] bool valid() const {
            return !nodes.empty();
        }
    };

    ImportedModel import_gltf(ResourceManager& resources, const std::filesystem::path& path);
} // namespace star::resources
