#include "star/resources/resource_manager.hpp"

#include <bgfx/bgfx.h>

#include "star/core/common.hpp"
#include "star/graphics/buffer.hpp"
#include "star/graphics/mesh.hpp"
#include "star/graphics/texture.hpp"
#include "star/resources/shader/shader.hpp"
#include "star/resources/shader/shaders.hpp"
#include "star/utils/file_utils.hpp"

namespace star::resources {
    static ResourceStorage<Mesh> s_meshes;
    static ResourceStorage<Texture> s_textures;
    static ResourceStorage<Shader> s_shaders;
    static ResourceStorage<Material> s_materials;

    ResourceManager::ResourceManager(Device& device) : m_device(device) {
        STAR_LOG_INFO(LogCategory::Resources, "Initializing ResourceManager");

        init_default_resources();
    }

    ResourceHandle<Mesh> ResourceManager::load_mesh(const std::string& path) {
        if (const auto it = s_meshes.path_to_id.find(path); it != s_meshes.path_to_id.end()) {
            const auto& entry = s_meshes.resources[it->second];
            return ResourceHandle<Mesh>{it->second, entry.generation};
        }

        // TODO: Load mesh from file
        STAR_ASSERT(false, "Mesh loading from file not yet implemented: {}", path);

        return {};
    }

    ResourceHandle<Mesh> ResourceManager::create_mesh(const std::string& name, std::unique_ptr<Mesh> mesh) const {
        if (!mesh) {
            STAR_LOG_ERROR(LogCategory::Resources, "Cannot create mesh with null pointer");
            return {};
        }

        if (const auto it = s_meshes.path_to_id.find(name); it != s_meshes.path_to_id.end()) {
            STAR_LOG_WARN(LogCategory::Resources, "Mesh '{}' already exists", name);
            const auto& entry = s_meshes.resources[it->second];
            return ResourceHandle<Mesh>{it->second, entry.generation};
        }

        upload_mesh_to_gpu(*mesh);

        const u32 id = s_meshes.allocate_id();
        mesh->m_path = name;
        mesh->m_state = ResourceState::Loaded;
        mesh->m_generation = 1;

        s_meshes.resources[id] = {std::move(mesh), 1, 1};
        s_meshes.path_to_id[name] = id;

        STAR_LOG_INFO(LogCategory::Resources, "Created mesh '{}' (id: {})", name, id);
        return ResourceHandle<Mesh>{id, 1};
    }

    Mesh* ResourceManager::get_mesh(const ResourceHandle<Mesh>& handle) {
        if (!handle.is_valid()) {
            return nullptr;
        }

        const auto it = s_meshes.resources.find(handle.id);
        if (it == s_meshes.resources.end()) {
            return nullptr;
        }

        if (it->second.generation != handle.generation) {
            return nullptr;
        }

        return it->second.resource.get();
    }

    void ResourceManager::destroy_mesh(const ResourceHandle<Mesh>& handle) {
        if (!handle.is_valid()) {
            return;
        }

        const auto it = s_meshes.resources.find(handle.id);
        if (it == s_meshes.resources.end()) {
            return;
        }

        const auto* mesh = it->second.resource.get();
        if (mesh->vertex_buffer.is_valid()) {
            m_device.destroy_buffer(mesh->vertex_buffer);
        }
        if (mesh->index_buffer.is_valid()) {
            m_device.destroy_buffer(mesh->index_buffer);
        }

        s_meshes.path_to_id.erase(mesh->path());

        s_meshes.resources.erase(handle.id);
        s_meshes.release_id(handle.id);

        STAR_LOG_DEBUG(LogCategory::Resources, "Destroyed mesh (id: {})", handle.id);
    }

    ResourceHandle<Texture> ResourceManager::load_texture(const std::string& path) {
        if (const auto it = s_textures.path_to_id.find(path); it != s_textures.path_to_id.end()) {
            const auto& entry = s_textures.resources[it->second];
            return ResourceHandle<Texture>{it->second, entry.generation};
        }

        // TODO: Load texture from file
        STAR_ASSERT(false, "Texture loading from file not yet implemented: {}", path);
        return {};
    }

    ResourceHandle<Texture> ResourceManager::create_texture(const std::string& name, std::unique_ptr<Texture> texture) {
        if (!texture) {
            STAR_LOG_ERROR(LogCategory::Resources, "Cannot create texture with null pointer");
            return {};
        }

        if (const auto it = s_textures.path_to_id.find(name); it != s_textures.path_to_id.end()) {
            STAR_LOG_WARN(LogCategory::Resources, "Texture '{}' already exists", name);
            const auto& entry = s_textures.resources[it->second];
            return ResourceHandle<Texture>{it->second, entry.generation};
        }

        upload_texture_to_gpu(*texture);

        const u32 id = s_textures.allocate_id();
        texture->m_path = name;
        texture->m_state = ResourceState::Loaded;
        texture->m_generation = 1;

        s_textures.resources[id] = {std::move(texture), 1, 1};
        s_textures.path_to_id[name] = id;

        STAR_LOG_INFO(LogCategory::Resources, "Created texture '{}' (id: {})", name, id);
        return ResourceHandle<Texture>{id, 1};
    }

    Texture* ResourceManager::get_texture(const ResourceHandle<Texture>& handle) {
        if (!handle.is_valid()) {
            return nullptr;
        }

        const auto it = s_textures.resources.find(handle.id);
        if (it == s_textures.resources.end()) {
            return nullptr;
        }

        if (it->second.generation != handle.generation) {
            return nullptr;
        }

        return it->second.resource.get();
    }

    void ResourceManager::destroy_texture(const ResourceHandle<Texture>& handle) {
        if (!handle.is_valid()) {
            return;
        }

        const auto it = s_textures.resources.find(handle.id);
        if (it == s_textures.resources.end()) {
            return;
        }

        const auto* texture = it->second.resource.get();
        if (texture->handle.is_valid()) {
            m_device.destroy_texture(texture->handle);
        }

        s_textures.path_to_id.erase(texture->path());

        s_textures.resources.erase(handle.id);
        s_textures.release_id(handle.id);

        STAR_LOG_DEBUG(LogCategory::Resources, "Destroyed texture (id: {})", handle.id);
    }

    ResourceHandle<graphics::Shader> ResourceManager::load_shader(const std::string& vertex_path,
                                                                  const std::string& fragment_path) const {
        const std::string shader_name = vertex_path + "+" + fragment_path;

        if (const auto it = s_shaders.path_to_id.find(shader_name); it != s_shaders.path_to_id.end()) {
            const auto& entry = s_shaders.resources[it->second];
            return ResourceHandle<graphics::Shader>{it->second, entry.generation};
        }

        std::string shader_dir = "glsl";
#if defined(_WIN32)
        shader_dir = "dx11";
#elif defined(__APPLE__)
        shader_dir = "metal";
#endif
        const std::string vs_compiled = vertex_path + "." + shader_dir + ".bin";
        const std::string fs_compiled = fragment_path + "." + shader_dir + ".bin";

        auto vs_bytecode = utils::read_binary_file(vs_compiled);
        if (vs_bytecode.empty()) {
            STAR_LOG_ERROR(LogCategory::Resources, "Failed to load vertex shader: {}", vs_compiled);
            return {};
        }

        auto fs_bytecode = utils::read_binary_file(fs_compiled);
        if (fs_bytecode.empty()) {
            STAR_LOG_ERROR(LogCategory::Resources, "Failed to load fragment shader: {}", fs_compiled);
            return {};
        }

        ShaderDescriptor desc;
        desc.name = shader_name;
        desc.stages = {{ShaderDescriptor::Stage::Vertex, std::move(vs_bytecode), "main"},
                       {ShaderDescriptor::Stage::Fragment, std::move(fs_bytecode), "main"}};

        const auto gpu_handle = m_device.create_shader(desc);
        if (!gpu_handle.is_valid()) {
            STAR_LOG_ERROR(LogCategory::Resources, "Failed to create GPU shader: {}", shader_name);
            return {};
        }

        auto shader = std::make_unique<Shader>();
        shader->handle = gpu_handle;
        shader->m_path = shader_name;
        shader->m_state = ResourceState::Loaded;
        shader->m_generation = 1;

        const u32 id = s_shaders.allocate_id();
        s_shaders.resources[id] = {std::move(shader), 1, 1};
        s_shaders.path_to_id[shader_name] = id;

        STAR_LOG_INFO(LogCategory::Resources, "Loaded shader '{}' (id: {})", shader_name, id);
        return ResourceHandle<graphics::Shader>{id, 1};
    }

    // I KNOW... I BROKE ALL THE LOGIC OF THE DEVICE BY DOING THIS LOL
    // I WILL FIX IT IN THE FUTURE, NOW IT IS 3 AM
    ResourceHandle<graphics::Shader>
    ResourceManager::create_embedded_shader(const std::string& name, const bgfx::EmbeddedShader& vertex_shader,
                                            const bgfx::EmbeddedShader& fragment_shader) {
        if (const auto it = s_shaders.path_to_id.find(name); it != s_shaders.path_to_id.end()) {
            const auto& entry = s_shaders.resources[it->second];
            return ResourceHandle<graphics::Shader>{it->second, entry.generation};
        }

        const auto caps = bgfx::getCaps();
        const bgfx::RendererType::Enum renderer_type = caps->rendererType;

        const auto vs_handle = bgfx::createEmbeddedShader(&vertex_shader, renderer_type, vertex_shader.name);
        if (!bgfx::isValid(vs_handle)) {
            STAR_LOG_ERROR(LogCategory::Resources, "Failed to create embedded vertex shader: {}", name);
            return {};
        }

        const auto fs_handle = bgfx::createEmbeddedShader(&fragment_shader, renderer_type, fragment_shader.name);
        if (!bgfx::isValid(fs_handle)) {
            STAR_LOG_ERROR(LogCategory::Resources, "Failed to create embedded fragment shader: {}", name);
            bgfx::destroy(vs_handle);
            return {};
        }

        const auto program = bgfx::createProgram(vs_handle, fs_handle, true);
        if (!bgfx::isValid(program)) {
            STAR_LOG_ERROR(LogCategory::Resources, "Failed to create shader program from embedded shaders: {}", name);
            return {};
        }

        auto shader = std::make_unique<Shader>();
        shader->handle = ResourceHandle<graphics::Shader>{program.idx, 0};
        shader->m_path = name;
        shader->m_state = ResourceState::Loaded;
        shader->m_generation = 1;

        const u32 id = s_shaders.allocate_id();
        s_shaders.resources[id] = {std::move(shader), 1, 1};
        s_shaders.path_to_id[name] = id;

        STAR_LOG_INFO(LogCategory::Resources, "Created embedded shader '{}' (id: {})", name, id);
        return ResourceHandle<graphics::Shader>{id, 1};
    }

    Shader* ResourceManager::get_shader(const ResourceHandle<graphics::Shader>& handle) {
        if (!handle.is_valid()) {
            return nullptr;
        }

        const auto it = s_shaders.resources.find(handle.id);
        if (it == s_shaders.resources.end()) {
            return nullptr;
        }

        if (it->second.generation != handle.generation) {
            return nullptr;
        }

        return it->second.resource.get();
    }

    ResourceHandle<Material> ResourceManager::create_material(const std::string& name,
                                                              std::unique_ptr<Material> material) {
        if (!material) {
            STAR_LOG_ERROR(LogCategory::Resources, "Cannot create material with null pointer");
            return {};
        }

        if (const auto it = s_materials.path_to_id.find(name); it != s_materials.path_to_id.end()) {
            STAR_LOG_WARN(LogCategory::Resources, "Material '{}' already exists", name);
            const auto& entry = s_materials.resources[it->second];
            return ResourceHandle<Material>{it->second, entry.generation};
        }

        const u32 id = s_materials.allocate_id();
        material->m_path = name;
        material->m_state = ResourceState::Loaded;
        material->m_generation = 1;

        s_materials.resources[id] = {std::move(material), 1, 1};
        s_materials.path_to_id[name] = id;

        STAR_LOG_INFO(LogCategory::Resources, "Created material '{}' (id: {})", name, id);
        return ResourceHandle<Material>{id, 1};
    }

    Material* ResourceManager::get_material(const ResourceHandle<Material>& handle) {
        if (!handle.is_valid()) {
            return nullptr;
        }

        const auto it = s_materials.resources.find(handle.id);
        if (it == s_materials.resources.end()) {
            return nullptr;
        }
        if (it->second.generation != handle.generation) {
            s_materials.resources.clear();
            s_materials.path_to_id.clear();
            s_materials.free_slots.clear();
            return nullptr;
        }

        return it->second.resource.get();
    }

    void ResourceManager::destroy_material(const ResourceHandle<Material>& handle) {
        if (!handle.is_valid()) {
            return;
        }

        if (const auto it = s_materials.resources.find(handle.id); it == s_materials.resources.end()) {
            return;
        }

        s_materials.resources.erase(handle.id);
        s_materials.release_id(handle.id);

        STAR_LOG_DEBUG(LogCategory::Resources, "Destroyed material (id: {})", handle.id);
    }

    void ResourceManager::destroy_all_resources() const {
        STAR_LOG_INFO(LogCategory::Resources, "Destroying all resources");

        for (const auto& entry : s_meshes.resources | std::views::values) {
            if (entry.resource->vertex_buffer.is_valid()) {
                m_device.destroy_buffer(entry.resource->vertex_buffer);
            }
            if (entry.resource->index_buffer.is_valid()) {
                m_device.destroy_buffer(entry.resource->index_buffer);
            }
        }
        s_meshes.resources.clear();
        s_meshes.path_to_id.clear();
        s_meshes.free_slots.clear();

        for (const auto& entry : s_textures.resources | std::views::values) {
            if (entry.resource->handle.is_valid()) {
                m_device.destroy_texture(entry.resource->handle);
            }
        }

        s_textures.resources.clear();
        s_textures.path_to_id.clear();
        s_textures.free_slots.clear();

        for (const auto& entry : s_shaders.resources | std::views::values) {
            if (entry.resource->handle.is_valid()) {
                m_device.destroy_shader(entry.resource->handle);
            }
        }

        s_shaders.resources.clear();
        s_shaders.path_to_id.clear();
        s_shaders.free_slots.clear();
    }

    void ResourceManager::garbage_collect() {
        // TODO: Implement reference counting and automatic cleanup
    }

    void ResourceManager::upload_mesh_to_gpu(Mesh& mesh) const {
        BufferDescriptor vertex_buffer_desc;
        vertex_buffer_desc.size_in_bytes = mesh.vertices.size() * sizeof(Vertex);
        vertex_buffer_desc.bind_flags = static_cast<u32>(BufferDescriptor::BindFlags::VertexBuffer);
        vertex_buffer_desc.usage = BufferDescriptor::Usage::Static;
        vertex_buffer_desc.type = BufferDescriptor::Type::Vertex;
        vertex_buffer_desc.initial_data = mesh.vertices.data();
        vertex_buffer_desc.stride = sizeof(Vertex);

        mesh.vertex_buffer = m_device.create_buffer(vertex_buffer_desc);

        if (!mesh.vertex_buffer.is_valid()) {
            STAR_LOG_ERROR(LogCategory::Resources, "Failed to create vertex buffer for mesh");
            return;
        }

        BufferDescriptor index_buffer_desc;
        index_buffer_desc.size_in_bytes = mesh.indices.size() * sizeof(u32);
        index_buffer_desc.bind_flags = static_cast<u32>(BufferDescriptor::BindFlags::IndexBuffer);
        index_buffer_desc.usage = BufferDescriptor::Usage::Static;
        index_buffer_desc.type = BufferDescriptor::Type::Index32;
        index_buffer_desc.initial_data = mesh.indices.data();

        mesh.index_buffer = m_device.create_buffer(index_buffer_desc);

        if (!mesh.index_buffer.is_valid()) {
            STAR_LOG_ERROR(LogCategory::Resources, "Failed to create index buffer for mesh");
            m_device.destroy_buffer(mesh.vertex_buffer);
            mesh.vertex_buffer = ResourceHandle<Buffer>{0, 0};
            return;
        }

        STAR_LOG_DEBUG(LogCategory::Resources, "Uploaded mesh to GPU (vertices: {}, indices: {})", mesh.vertices.size(),
                       mesh.indices.size());
    }

    void ResourceManager::upload_texture_to_gpu(Texture& texture) {
        // TODO: Implement texture upload
        STAR_ASSERT(false, "Texture upload not yet implemented");
    }

    void ResourceManager::init_default_resources() {
        STAR_LOG_INFO(LogCategory::Resources, "Creating default resources");

        auto cube = std::make_unique<Mesh>();
        *cube = Mesh::create_cube(1.0f);
        m_cube_mesh = create_mesh("__default_cube", std::move(cube));

        auto plane = std::make_unique<Mesh>();
        *plane = Mesh::create_plane();
        m_plane_mesh = create_mesh("__default_plane", std::move(plane));

        auto sphere = std::make_unique<Mesh>();
        *sphere = Mesh::create_sphere(1.0f, 32, 32);
        m_sphere_mesh = create_mesh("__default_sphere", std::move(sphere));

        m_default_shader = create_embedded_shader("__simple_shader", k_material_vs, k_material_fs);

        if (!m_default_shader.is_valid()) {
            STAR_LOG_ERROR(LogCategory::Resources, "Failed to create default embedded shader!");
        } else {
            auto default_mat = std::make_unique<Material>();
            default_mat->shader = m_default_shader;
            default_mat->albedo_color = Vector4{1.0f, 0.0f, 0.0f, 1.0f};
            m_default_material = create_material("__default_material", std::move(default_mat));

            STAR_LOG_INFO(LogCategory::Resources, "Default material created with embedded shader");
        }

        STAR_LOG_INFO(LogCategory::Resources, "Default resources created");
    }
} // namespace star::resources
