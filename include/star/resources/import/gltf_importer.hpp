#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "star/core/types.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/graphics/vertex.hpp"
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

    struct RawTexture {
        std::string name;
        u32 width{0};
        u32 height{0};
        std::vector<u8> pixels;
    };

    struct RawMaterial {
        std::string name;
        Vector4 albedo_color{1.0f, 1.0f, 1.0f, 1.0f};
        Vector4 emissive_color{0.0f, 0.0f, 0.0f, 1.0f};
        f32 metallic{0.0f};
        f32 roughness{0.5f};
        i32 albedo_texture{-1};
        i32 normal_texture{-1};
        i32 metallic_roughness_texture{-1};
        i32 emissive_texture{-1};
    };

    struct RawMesh {
        std::string name;
        std::vector<graphics::Vertex> vertices;
        std::vector<u32> indices;
    };

    struct RawPrimitive {
        i32 mesh{-1};
        i32 material{-1};
    };

    struct RawNode {
        std::string name;
        Vector3 position{0.0f, 0.0f, 0.0f};
        Quaternion rotation{0.0f, 0.0f, 0.0f, 1.0f};
        Vector3 scale{1.0f, 1.0f, 1.0f};
        std::vector<RawPrimitive> primitives;
        std::vector<u32> children;
    };

    struct RawModel {
        std::string name;
        std::vector<RawTexture> textures;
        std::vector<RawMaterial> materials;
        std::vector<RawMesh> meshes;
        std::vector<RawNode> nodes;
        std::vector<u32> roots;
        bool ok{false};
    };

    RawModel parse_gltf(const std::filesystem::path& path);

    ImportedModel upload_gltf(ResourceManager& resources, const RawModel& raw);

    ImportedModel import_gltf(ResourceManager& resources, const std::filesystem::path& path);
} // namespace star::resources
