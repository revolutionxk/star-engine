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
#include "star/core/uuid.hpp"
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
    struct Shader;

    template<typename T>
    struct ResourceStorage {
        struct Entry {
            std::unique_ptr<T> resource;
            std::string name;
            UUID uuid;
            u32 generation = 1;
            u32 ref_count = 0;
        };

        std::unordered_map<u32, Entry> resources;
        std::unordered_map<std::string, u32> path_to_id;
        std::unordered_map<UUID, u32> uuid_to_id;
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

        [[nodiscard]] UUID uuid_of(const u32 id) const {
            const auto it = resources.find(id);
            return it != resources.end() ? it->second.uuid : UUID{};
        }

        [[nodiscard]] graphics::ResourceHandle<T> by_uuid(const UUID& uuid) const {
            const auto it = uuid_to_id.find(uuid);
            if (it == uuid_to_id.end()) {
                return {};
            }
            const auto entry = resources.find(it->second);
            return graphics::ResourceHandle<T>{it->second, entry != resources.end() ? entry->second.generation : 1};
        }
    };

    class ResourceManager {
      public:
        explicit ResourceManager(graphics::Device& device);
        ~ResourceManager();

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        graphics::ResourceHandle<Mesh> create_mesh(const std::string& name, std::unique_ptr<Mesh> mesh, UUID uuid = {});
        Mesh* get_mesh(const graphics::ResourceHandle<Mesh>& handle);
        void destroy_mesh(const graphics::ResourceHandle<Mesh>& handle);

        graphics::ResourceHandle<Texture> load_texture(const std::string& path);
        graphics::ResourceHandle<Texture> load_environment(const std::string& path);
        graphics::ResourceHandle<Texture> get_or_load_texture(const std::string& name);
        graphics::ResourceHandle<Texture> create_texture(const std::string& name, std::unique_ptr<Texture> texture, UUID uuid = {});
        Texture* get_texture(const graphics::ResourceHandle<Texture>& handle);
        void destroy_texture(const graphics::ResourceHandle<Texture>& handle);
        graphics::ResourceHandle<Material> create_material(const std::string& name, std::unique_ptr<Material> material, UUID uuid = {});
        Material* get_material(const graphics::ResourceHandle<Material>& handle);
        void destroy_material(const graphics::ResourceHandle<Material>& handle);

        void set_asset_root(std::filesystem::path root);
        graphics::ResourceHandle<Material> load_material(const std::string& relative_path);
        graphics::ResourceHandle<Material> get_or_load_material(const std::string& name);
        bool save_material(const graphics::ResourceHandle<Material>& handle, const std::string& relative_path);

        graphics::ResourceHandle<graphics::Shader> load_shader(const std::string& vertex_path, const std::string& fragment_path);
        graphics::ResourceHandle<graphics::Shader> register_builtin_shader(const std::string& name, BuiltinShader id);
        Shader* get_shader(const graphics::ResourceHandle<graphics::Shader>& handle);

        bool reload_shader_from_disk(const graphics::ResourceHandle<graphics::Shader>& handle);

        void destroy_all_resources();
        void garbage_collect();

        [[nodiscard]] std::string mesh_name(const graphics::ResourceHandle<Mesh>& handle) const;
        [[nodiscard]] graphics::ResourceHandle<Mesh> mesh_by_name(const std::string& name) const;
        [[nodiscard]] std::string texture_name(const graphics::ResourceHandle<Texture>& handle) const;
        [[nodiscard]] graphics::ResourceHandle<Texture> texture_by_name(const std::string& name) const;
        [[nodiscard]] std::string material_name(const graphics::ResourceHandle<Material>& handle) const;
        [[nodiscard]] graphics::ResourceHandle<Material> material_by_name(const std::string& name) const;
        [[nodiscard]] std::string shader_name(const graphics::ResourceHandle<graphics::Shader>& handle) const;

        [[nodiscard]] UUID mesh_uuid(const graphics::ResourceHandle<Mesh>& handle) const;
        [[nodiscard]] UUID texture_uuid(const graphics::ResourceHandle<Texture>& handle) const;
        [[nodiscard]] UUID material_uuid(const graphics::ResourceHandle<Material>& handle) const;

        [[nodiscard]] graphics::ResourceHandle<Mesh> mesh_by_uuid(const UUID& uuid) const;
        [[nodiscard]] graphics::ResourceHandle<Texture> texture_by_uuid(const UUID& uuid) const;
        [[nodiscard]] graphics::ResourceHandle<Material> material_by_uuid(const UUID& uuid) const;
        [[nodiscard]] graphics::ResourceHandle<graphics::Shader> shader_by_name(const std::string& name) const;

        graphics::ResourceHandle<Mesh> cube_mesh() const {
            return m_cube_mesh;
        }

        graphics::ResourceHandle<Mesh> sphere_mesh() const {
            return m_sphere_mesh;
        }

        graphics::ResourceHandle<Material> default_material() const {
            return m_default_material;
        }

        graphics::ResourceHandle<Mesh> plane_mesh() const {
            return m_plane_mesh;
        }

        graphics::ResourceHandle<Texture> white_texture() const {
            return m_white_texture;
        }

        graphics::ResourceHandle<Texture> black_texture() const {
            return m_black_texture;
        }

        graphics::ResourceHandle<graphics::Shader> default_shader() const {
            return m_default_shader;
        }

      private:
        void upload_mesh_to_gpu(Mesh& mesh) const;
        void upload_texture_to_gpu(Texture& texture);
        void init_default_resources();

        graphics::Device& m_device;

        ResourceStorage<Mesh> m_meshes;
        ResourceStorage<Texture> m_textures;
        ResourceStorage<Shader> m_shaders;
        ResourceStorage<Material> m_materials;

        graphics::ResourceHandle<Mesh> m_cube_mesh;
        graphics::ResourceHandle<Mesh> m_sphere_mesh;
        graphics::ResourceHandle<Mesh> m_plane_mesh;
        graphics::ResourceHandle<Texture> m_white_texture;
        graphics::ResourceHandle<Texture> m_black_texture;
        graphics::ResourceHandle<Material> m_default_material;
        graphics::ResourceHandle<graphics::Shader> m_default_shader;

        std::filesystem::path m_asset_root;
    };
} // namespace star::resources
