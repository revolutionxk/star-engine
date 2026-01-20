#pragma once
#include "mesh/mesh.hpp"
#include "star/core/types.hpp"
#include "star/graphics/buffer.hpp"
#include "star/graphics/device.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/graphics/shader.hpp"
#include "texture/texture.hpp"

namespace star::graphics {
    struct Mesh;
    struct Texture;
    struct Shader;
} // namespace star::graphics

namespace star::resources {
    using namespace star::graphics;

    class ResourceManager {
      public:
        explicit ResourceManager(Device& device);
        ~ResourceManager() = default;

        ResourceHandle<Mesh> load_mesh(const std::string& path);
        ResourceHandle<Mesh> create_mesh(const std::string& name, std::unique_ptr<Mesh> mesh);
        Mesh* get_mesh(const ResourceHandle<Mesh>& handle);
        void destroy_mesh(const ResourceHandle<Mesh>& handle);

        ResourceHandle<Texture> load_texture(const std::string& path);
        ResourceHandle<Texture> create_texture(const std::string& name, std::unique_ptr<Texture> texture);
        Texture* get_texture(const ResourceHandle<Texture>& handle);
        void destroy_texture(const ResourceHandle<Texture>& handle);

        ResourceHandle<Shader> load_shader(const std::string& vertex_path, const std::string& fragment_path);

        void destroy_all_resources();
        void garbage_collect();

      private:
        void upload_mesh_to_gpu(Mesh& mesh) const;
        void upload_texture_to_gpu(Texture& texture);
        void init_default_resources();

        template<typename T>
        struct ResourceStorage {
            struct Entry {
                std::unique_ptr<T> resource;
                u32 generation = 1;
                u32 ref_count = 0;
            };

            std::unordered_map<u32, Entry> resources;
            std::unordered_map<std::string, u32> path_to_id;
            std::vector<u32> free_slots;
            u32 next_id = 1;

            u32 allocate_id() {
                if (!free_slots.empty()) {
                    const auto id = free_slots.back();
                    free_slots.pop_back();
                    return id;
                }
                return next_id++;
            }

            void release_id(const u32 id) {
                free_slots.push_back(id);
            }
        };

        Device& m_device;

        ResourceHandle<Mesh> m_cube_mesh;
        ResourceHandle<Mesh> m_sphere_mesh;
        ResourceHandle<Mesh> m_plane_mesh;
        ResourceHandle<Texture> m_white_texture;
        ResourceHandle<Texture> m_black_texture;
    };
} // namespace star::resources
