#include "star/resources/mesh/mesh.hpp"

namespace star::resources {
    Mesh Mesh::create_cube(float size) {
        Mesh mesh;

        mesh.vertices = {
            // Front face
            {{-size, -size, size}, {0, 0, 1}, {0, 0}, {1, 0, 0}, {0, 1, 0}},
            {{size, -size, size}, {0, 0, 1}, {1, 0}, {1, 0, 0}, {0, 1, 0}},
            {{size, size, size}, {0, 0, 1}, {1, 1}, {1, 0, 0}, {0, 1, 0}},
            {{-size, size, size}, {0, 0, 1}, {0, 1}, {1, 0, 0}, {0, 1, 0}},
            // Back face
            {{-size, -size, -size}, {0, 0, -1}, {1, 0}, {-1, 0, 0}, {0, 1, 0}},
            {{-size, size, -size}, {0, 0, -1}, {1, 1}, {-1, 0, 0}, {0, 1, 0}},
            {{size, size, -size}, {0, 0, -1}, {0, 1}, {-1, 0, 0}, {0, 1, 0}},
            {{size, -size, -size}, {0, 0, -1}, {0, 0}, {-1, 0, 0}, {0, 1, 0}},
            // Left face
            {{-size, -size, -size}, {-1, 0, 0}, {0, 0}, {0, 0, -1}, {0, 1, 0}},
            {{-size, -size, size}, {-1, 0, 0}, {1, 0}, {0, 0, -1}, {0, 1, 0}},
            {{-size, size, size}, {-1, 0, 0}, {1, 1}, {0, 0, -1}, {0, 1, 0}},
            {{-size, size, -size}, {-1, 0, 0}, {0, 1}, {0, 0, -1}, {0, 1, 0}},
            // Right face
            {{size, -size, -size}, {1, 0, 0}, {1, 0}, {0, 0, 1}, {0, 1, 0}},
            {{size, size, -size}, {1, 0, 0}, {1, 1}, {0, 0, 1}, {0, 1, 0}},
            {{size, size, size}, {1, 0, 0}, {0, 1}, {0, 0, 1}, {0, 1, 0}},
            {{size, -size, size}, {1, 0, 0}, {0, 0}, {0, 0, 1}, {0, 1, 0}},
            // Top face
            {{-size, size, -size}, {0, 1, 0}, {0, 1}, {1, 0, 0}, {0, 0, -1}},
            {{-size, size, size}, {0, 1, 0}, {0, 0}, {1, 0, 0}, {0, 0, -1}},
            {{size, size, size}, {0, 1, 0}, {1, 0}, {1, 0, 0}, {0, 0, -1}},
            {{size, size, -size}, {0, 1, 0}, {1, 1}, {1, 0, 0}, {0, 0, -1}},
            // Bottom face
            {{-size, -size, -size}, {0, -1, 0}, {1, 1}, {1, 0, 0}, {0, 0, 1}},
            {{size, -size, -size}, {0, -1, 0}, {0, 1}, {1, 0, 0}, {0, 0, 1}},
            {{size, -size, size}, {0, -1, 0}, {0, 0}, {1, 0, 0}, {0, 0, 1}},
            {{-size, -size, size}, {0, -1, 0}, {1, 0}, {1, 0, 0}, {0, 0, 1}},
        };

        mesh.indices = {
            0,  1,  2,  2,  3,  0,  // Front face
            4,  5,  6,  6,  7,  4,  // Back face
            8,  9,  10, 10, 11, 8,  // Left face
            12, 13, 14, 14, 15, 12, // Right face
            16, 17, 18, 18, 19, 16, // Top face
            20, 21, 22, 22, 23, 20  // Bottom face
        };

        return mesh;
    }

    Mesh Mesh::create_sphere(const float radius, const u32 segments, const u32 rings) {
        Mesh mesh;

        for (u32 y = 0; y <= rings; ++y) {
            for (u32 x = 0; x <= segments; ++x) {
                const float x_segment = static_cast<float>(x) / static_cast<float>(segments);
                const float y_segment = static_cast<float>(y) / static_cast<float>(rings);
                const float x_pos = radius * std::cos(x_segment * 2.0f * math::Constantsf::pi) *
                                    std::sin(y_segment * math::Constantsf::pi);
                const float y_pos = radius * std::cos(y_segment * math::Constantsf::pi);
                const float z_pos = radius * std::sin(x_segment * 2.0f * math::Constantsf::pi) *
                                    std::sin(y_segment * math::Constantsf::pi);

                Vector3 position = {x_pos, y_pos, z_pos};
                Vector3 normal = position.normalized();
                const Vector2 tex_coord = {x_segment, y_segment};
                Vector3 tangent = {-std::sin(x_segment * 2.0f * math::Constantsf::pi), 0.0f,
                                   std::cos(x_segment * 2.0f * math::Constantsf::pi)};
                const Vector3 bitangent = normal.cross(tangent);

                mesh.vertices.push_back({position, normal, tex_coord, tangent, bitangent});
            }
        }

        for (u32 y = 0; y < rings; ++y) {
            for (u32 x = 0; x < segments; ++x) {
                u32 first = (y * (segments + 1)) + x;
                u32 second = first + segments + 1;

                mesh.indices.push_back(first);
                mesh.indices.push_back(second);
                mesh.indices.push_back(first + 1);

                mesh.indices.push_back(second);
                mesh.indices.push_back(second + 1);
                mesh.indices.push_back(first + 1);
            }
        }

        return mesh;
    }

    Mesh Mesh::create_plane() {
        Mesh mesh;
        mesh.vertices = {
            {{-0.5f, 0.0f, -0.5f}, {0, 1, 0}, {0, 0}, {1, 0, 0}, {0, 0, 1}},
            {{0.5f, 0.0f, -0.5f}, {0, 1, 0}, {1, 0}, {1, 0, 0}, {0, 0, 1}},
            {{0.5f, 0.0f, 0.5f}, {0, 1, 0}, {1, 1}, {1, 0, 0}, {0, 0, 1}},
            {{-0.5f, 0.0f, 0.5f}, {0, 1, 0}, {0, 1}, {1, 0, 0}, {0, 0, 1}},
        };

        mesh.indices = {0, 1, 2, 2, 3, 0};

        return mesh;
    }
} // namespace star::resources
