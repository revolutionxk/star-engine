#pragma once
#include <vector>

#include "star/graphics/resource_handle.hpp"
#include "star/graphics/vertex.hpp"
#include "star/math/math.hpp"
#include "star/resources/resource.hpp"

namespace star::graphics {
    struct Buffer;
} // namespace star::graphics

namespace star::resources {
    struct Mesh : Resource {
        std::vector<Vertex> vertices{};
        std::vector<u32> indices{};

        graphics::ResourceHandle<graphics::Buffer> vertex_buffer;
        graphics::ResourceHandle<graphics::Buffer> index_buffer;

        u32 vertex_count() const {
            return static_cast<u32>(vertices.size());
        }

        u32 index_count() const {
            return static_cast<u32>(indices.size());
        }

        AABB bounding_box() const {
            if (vertices.empty()) {
                return AABB{};
            }

            Vector3 min = vertices[0].position;
            Vector3 max = vertices[0].position;

            for (const auto& vertex : vertices) {
                min.x = std::min(min.x, vertex.position.x);
                min.y = std::min(min.y, vertex.position.y);
                min.z = std::min(min.z, vertex.position.z);

                max.x = std::max(max.x, vertex.position.x);
                max.y = std::max(max.y, vertex.position.y);
                max.z = std::max(max.z, vertex.position.z);
            }

            return AABB{min, max};
        }

        static Mesh create_cube(float size);
        static Mesh create_sphere(float radius, u32 segments, u32 rings);
        static Mesh create_plane();
    };
} // namespace star::resources
