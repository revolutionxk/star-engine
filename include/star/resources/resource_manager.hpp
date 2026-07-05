#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "material/material.hpp"
#include "mesh/mesh.hpp"
#include "shader/builtin_shaders.hpp"
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

    struct Shader;

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
        std::unordered_map<u32, u32> generations;
        u32 next_id = 1;

        u32 allocate_id() {
            if (!free_slots.empty()) {
                const auto id = free_slots.back();
                free_slots.pop_back();
                return id;
            }
            return next_id++;
        }

        [[nodiscard]] u32 generation_of(const u32 id) const {
            const auto it = generations.find(id);
            return it != generations.end() ? it->second : 1;
        }

        void release_id(const u32 id) {
            generations[id] = generation_of(id) + 1;
            free_slots.push_back(id);
        }
    };

    class ResourceManager {
      public:
        explicit ResourceManager(Device& device);
        ~ResourceManager();

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        ResourceHandle<Mesh> load_mesh(const std::string& path);
        ResourceHandle<Mesh> create_mesh(const std::string& name, std::unique_ptr<Mesh> mesh);
        Mesh* get_mesh(const ResourceHandle<Mesh>& handle);
        void destroy_mesh(const ResourceHandle<Mesh>& handle);

        ResourceHandle<Texture> load_texture(const std::string& path);
        ResourceHandle<Texture> create_texture(const std::string& name, std::unique_ptr<Texture> texture);
        Texture* get_texture(const ResourceHandle<Texture>& handle);
        void destroy_texture(const ResourceHandle<Texture>& handle);
        ResourceHandle<Material> create_material(const std::string& name, std::unique_ptr<Material> material);
        Material* get_material(const ResourceHandle<Material>& handle);
        void destroy_material(const ResourceHandle<Material>& handle);

        void set_asset_root(std::filesystem::path root);
        ResourceHandle<Material> load_material(const std::string& relative_path);
        ResourceHandle<Material> get_or_load_material(const std::string& name);
        bool save_material(const ResourceHandle<Material>& handle, const std::string& relative_path);

        ResourceHandle<graphics::Shader> load_shader(const std::string& vertex_path, const std::string& fragment_path);
        ResourceHandle<graphics::Shader> register_builtin_shader(const std::string& name, BuiltinShader id);
        Shader* get_shader(const ResourceHandle<graphics::Shader>& handle);

        bool reload_shader_from_disk(const ResourceHandle<graphics::Shader>& handle);

        void destroy_all_resources();
        void garbage_collect();

        [[nodiscard]] std::string mesh_name(const ResourceHandle<Mesh>& handle) const;
        [[nodiscard]] ResourceHandle<Mesh> mesh_by_name(const std::string& name) const;
        [[nodiscard]] std::string texture_name(const ResourceHandle<Texture>& handle) const;
        [[nodiscard]] ResourceHandle<Texture> texture_by_name(const std::string& name) const;
        [[nodiscard]] std::string material_name(const ResourceHandle<Material>& handle) const;
        [[nodiscard]] ResourceHandle<Material> material_by_name(const std::string& name) const;
        [[nodiscard]] std::string shader_name(const ResourceHandle<graphics::Shader>& handle) const;
        [[nodiscard]] ResourceHandle<graphics::Shader> shader_by_name(const std::string& name) const;

        ResourceHandle<Mesh> cube_mesh() const {
            return m_cube_mesh;
        }

        ResourceHandle<Mesh> sphere_mesh() const {
            return m_sphere_mesh;
        }

        ResourceHandle<Material> default_material() const {
            return m_default_material;
        }

        ResourceHandle<Mesh> plane_mesh() const {
            return m_plane_mesh;
        }

        ResourceHandle<Texture> white_texture() const {
            return m_white_texture;
        }

        ResourceHandle<Texture> black_texture() const {
            return m_black_texture;
        }

        ResourceHandle<graphics::Shader> default_shader() const {
            return m_default_shader;
        }

      private:
        void upload_mesh_to_gpu(Mesh& mesh) const;
        void upload_texture_to_gpu(Texture& texture);
        void init_default_resources();

        Device& m_device;

        ResourceStorage<Mesh> m_meshes;
        ResourceStorage<Texture> m_textures;
        ResourceStorage<Shader> m_shaders;
        ResourceStorage<Material> m_materials;

        ResourceHandle<Mesh> m_cube_mesh;
        ResourceHandle<Mesh> m_sphere_mesh;
        ResourceHandle<Mesh> m_plane_mesh;
        ResourceHandle<Texture> m_white_texture;
        ResourceHandle<Texture> m_black_texture;
        ResourceHandle<Material> m_default_material;
        ResourceHandle<graphics::Shader> m_default_shader;

        std::filesystem::path m_asset_root;
    };
} // namespace star::resources
