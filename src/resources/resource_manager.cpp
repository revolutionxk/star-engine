#include "star/resources/resource_manager.hpp"

#include "shader/builtin_shader_registry.hpp"
#include "star/core/common.hpp"
#include "star/graphics/buffer.hpp"
#include "star/graphics/mesh.hpp"
#include "star/graphics/texture.hpp"
#include "star/resources/shader/shader.hpp"
#include "star/utils/file_utils.hpp"

namespace star::resources {

    ResourceManager::ResourceManager(Device& device) : m_device(device) {
        STAR_LOG_INFO(LogCategory::Resources, "Initializing ResourceManager");

        init_default_resources();
    }

    ResourceManager::~ResourceManager() {
        if (!m_meshes.resources.empty() || !m_textures.resources.empty() || !m_shaders.resources.empty() ||
            !m_materials.resources.empty()) {
            destroy_all_resources();
        }
    }

    ResourceHandle<Mesh> ResourceManager::load_mesh(const std::string& path) {
        if (const auto it = m_meshes.path_to_id.find(path); it != m_meshes.path_to_id.end()) {
            const auto& entry = m_meshes.resources[it->second];
            return ResourceHandle<Mesh>{it->second, entry.generation};
        }

        // TODO: Load mesh from file
        STAR_ASSERT(false, "Mesh loading from file not yet implemented: {}", path);

        return {};
    }

    ResourceHandle<Mesh> ResourceManager::create_mesh(const std::string& name, std::unique_ptr<Mesh> mesh) {
        if (!mesh) {
            STAR_LOG_ERROR(LogCategory::Resources, "Cannot create mesh with null pointer");
            return {};
        }

        if (const auto it = m_meshes.path_to_id.find(name); it != m_meshes.path_to_id.end()) {
            STAR_LOG_WARN(LogCategory::Resources, "Mesh '{}' already exists", name);
            const auto& entry = m_meshes.resources[it->second];
            return ResourceHandle<Mesh>{it->second, entry.generation};
        }

        upload_mesh_to_gpu(*mesh);

        const u32 id = m_meshes.allocate_id();
        mesh->m_path = name;
        mesh->m_state = ResourceState::Loaded;
        mesh->m_generation = 1;

        m_meshes.resources[id] = {std::move(mesh), 1, 1};
        m_meshes.path_to_id[name] = id;

        STAR_LOG_INFO(LogCategory::Resources, "Created mesh '{}' (id: {})", name, id);
        return ResourceHandle<Mesh>{id, 1};
    }

    Mesh* ResourceManager::get_mesh(const ResourceHandle<Mesh>& handle) {
        if (!handle.is_valid()) {
            return nullptr;
        }

        const auto it = m_meshes.resources.find(handle.id);
        if (it == m_meshes.resources.end() || it->second.generation != handle.generation) {
            return nullptr;
        }

        return it->second.resource.get();
    }

    void ResourceManager::destroy_mesh(const ResourceHandle<Mesh>& handle) {
        if (!handle.is_valid()) {
            return;
        }

        const auto it = m_meshes.resources.find(handle.id);
        if (it == m_meshes.resources.end() || it->second.generation != handle.generation) {
            return;
        }

        const auto* mesh = it->second.resource.get();
        if (mesh->vertex_buffer.is_valid()) {
            m_device.destroy_buffer(mesh->vertex_buffer);
        }
        if (mesh->index_buffer.is_valid()) {
            m_device.destroy_buffer(mesh->index_buffer);
        }

        m_meshes.path_to_id.erase(mesh->path());
        m_meshes.resources.erase(it);
        m_meshes.release_id(handle.id);

        STAR_LOG_DEBUG(LogCategory::Resources, "Destroyed mesh (id: {})", handle.id);
    }

    ResourceHandle<Texture> ResourceManager::load_texture(const std::string& path) {
        if (const auto it = m_textures.path_to_id.find(path); it != m_textures.path_to_id.end()) {
            const auto& entry = m_textures.resources[it->second];
            return ResourceHandle<Texture>{it->second, entry.generation};
        }

        // TODO: Load texture from file
        STAR_LOG_WARN(LogCategory::Resources, "Texture loading from file not yet implemented: {}", path);
        return {};
    }

    ResourceHandle<Texture> ResourceManager::create_texture(const std::string& name, std::unique_ptr<Texture> texture) {
        if (!texture) {
            STAR_LOG_ERROR(LogCategory::Resources, "Cannot create texture with null pointer");
            return {};
        }

        if (const auto it = m_textures.path_to_id.find(name); it != m_textures.path_to_id.end()) {
            STAR_LOG_WARN(LogCategory::Resources, "Texture '{}' already exists", name);
            const auto& entry = m_textures.resources[it->second];
            return ResourceHandle<Texture>{it->second, entry.generation};
        }

        upload_texture_to_gpu(*texture);

        const u32 id = m_textures.allocate_id();
        texture->m_path = name;
        texture->m_state = ResourceState::Loaded;
        texture->m_generation = 1;

        m_textures.resources[id] = {std::move(texture), 1, 1};
        m_textures.path_to_id[name] = id;

        STAR_LOG_INFO(LogCategory::Resources, "Created texture '{}' (id: {})", name, id);
        return ResourceHandle<Texture>{id, 1};
    }

    Texture* ResourceManager::get_texture(const ResourceHandle<Texture>& handle) {
        if (!handle.is_valid()) {
            return nullptr;
        }

        const auto it = m_textures.resources.find(handle.id);
        if (it == m_textures.resources.end() || it->second.generation != handle.generation) {
            return nullptr;
        }

        return it->second.resource.get();
    }

    void ResourceManager::destroy_texture(const ResourceHandle<Texture>& handle) {
        if (!handle.is_valid()) {
            return;
        }

        const auto it = m_textures.resources.find(handle.id);
        if (it == m_textures.resources.end() || it->second.generation != handle.generation) {
            return;
        }

        const auto* texture = it->second.resource.get();
        if (texture->handle.is_valid()) {
            m_device.destroy_texture(texture->handle);
        }

        m_textures.path_to_id.erase(texture->path());
        m_textures.resources.erase(it);
        m_textures.release_id(handle.id);

        STAR_LOG_DEBUG(LogCategory::Resources, "Destroyed texture (id: {})", handle.id);
    }

    ResourceHandle<graphics::Shader> ResourceManager::load_shader(const std::string& vertex_path,
                                                                  const std::string& fragment_path) {
        const std::string shader_name = vertex_path + "+" + fragment_path;

        if (const auto it = m_shaders.path_to_id.find(shader_name); it != m_shaders.path_to_id.end()) {
            const auto& entry = m_shaders.resources[it->second];
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
        shader->disk_vertex_path = vs_compiled;
        shader->disk_fragment_path = fs_compiled;
        shader->m_state = ResourceState::Loaded;
        shader->m_generation = 1;

        const u32 id = m_shaders.allocate_id();
        m_shaders.resources[id] = {std::move(shader), 1, 1};
        m_shaders.path_to_id[shader_name] = id;

        STAR_LOG_INFO(LogCategory::Resources, "Loaded shader '{}' (id: {})", shader_name, id);
        return ResourceHandle<graphics::Shader>{id, 1};
    }

    ResourceHandle<graphics::Shader> ResourceManager::register_builtin_shader(const std::string& name,
                                                                              const BuiltinShader builtin) {
        if (const auto it = m_shaders.path_to_id.find(name); it != m_shaders.path_to_id.end()) {
            const auto& entry = m_shaders.resources[it->second];
            return ResourceHandle<graphics::Shader>{it->second, entry.generation};
        }

        const auto gpu_handle = detail::create_embedded_program(builtin, name);
        if (!gpu_handle.is_valid()) {
            STAR_LOG_ERROR(LogCategory::Resources, "Failed to create embedded shader '{}'", name);
            return {};
        }

        auto shader = std::make_unique<Shader>();
        shader->handle = gpu_handle;
        shader->m_path = name;
        shader->m_state = ResourceState::Loaded;
        shader->m_generation = 1;

        const u32 id = m_shaders.allocate_id();
        m_shaders.resources[id] = {std::move(shader), 1, 1};
        m_shaders.path_to_id[name] = id;

        STAR_LOG_INFO(LogCategory::Resources, "Registered builtin shader '{}' (id: {})", name, id);
        return ResourceHandle<graphics::Shader>{id, 1};
    }

    Shader* ResourceManager::get_shader(const ResourceHandle<graphics::Shader>& handle) {
        if (!handle.is_valid()) {
            return nullptr;
        }

        const auto it = m_shaders.resources.find(handle.id);
        if (it == m_shaders.resources.end() || it->second.generation != handle.generation) {
            return nullptr;
        }

        return it->second.resource.get();
    }

    bool ResourceManager::reload_shader_from_disk(const ResourceHandle<graphics::Shader>& handle) {
        Shader* shader = get_shader(handle);
        if (!shader) {
            STAR_LOG_WARN(LogCategory::Resources, "reload_shader_from_disk: invalid handle");
            return false;
        }

        if (shader->disk_vertex_path.empty() || shader->disk_fragment_path.empty()) {
            STAR_LOG_WARN(LogCategory::Resources,
                          "reload_shader_from_disk: shader '{}' has no disk paths (embedded shaders cannot reload)",
                          shader->m_path);
            return false;
        }

        auto vs_bytecode = utils::read_binary_file(shader->disk_vertex_path);
        auto fs_bytecode = utils::read_binary_file(shader->disk_fragment_path);
        if (vs_bytecode.empty() || fs_bytecode.empty()) {
            STAR_LOG_ERROR(LogCategory::Resources, "reload_shader_from_disk: failed to read bytecode for '{}'",
                           shader->m_path);
            return false;
        }

        ShaderDescriptor desc;
        desc.name = shader->m_path;
        desc.stages = {{ShaderDescriptor::Stage::Vertex, std::move(vs_bytecode), "main"},
                       {ShaderDescriptor::Stage::Fragment, std::move(fs_bytecode), "main"}};

        const auto new_gpu = m_device.reload_shader(shader->handle, desc);
        if (!new_gpu.is_valid()) {
            STAR_LOG_ERROR(LogCategory::Resources, "reload_shader_from_disk: GPU reload failed for '{}'",
                           shader->m_path);
            return false;
        }

        shader->handle = new_gpu;
        STAR_LOG_INFO(LogCategory::Resources, "Hot-reloaded shader '{}'", shader->m_path);
        return true;
    }

    ResourceHandle<Material> ResourceManager::create_material(const std::string& name,
                                                              std::unique_ptr<Material> material) {
        if (!material) {
            STAR_LOG_ERROR(LogCategory::Resources, "Cannot create material with null pointer");
            return {};
        }

        if (const auto it = m_materials.path_to_id.find(name); it != m_materials.path_to_id.end()) {
            STAR_LOG_WARN(LogCategory::Resources, "Material '{}' already exists", name);
            const auto& entry = m_materials.resources[it->second];
            return ResourceHandle<Material>{it->second, entry.generation};
        }

        const u32 id = m_materials.allocate_id();
        material->m_path = name;
        material->m_state = ResourceState::Loaded;
        material->m_generation = 1;

        m_materials.resources[id] = {std::move(material), 1, 1};
        m_materials.path_to_id[name] = id;

        STAR_LOG_INFO(LogCategory::Resources, "Created material '{}' (id: {})", name, id);
        return ResourceHandle<Material>{id, 1};
    }

    Material* ResourceManager::get_material(const ResourceHandle<Material>& handle) {
        if (!handle.is_valid()) {
            return nullptr;
        }

        const auto it = m_materials.resources.find(handle.id);
        if (it == m_materials.resources.end() || it->second.generation != handle.generation) {
            return nullptr;
        }

        return it->second.resource.get();
    }

    void ResourceManager::destroy_material(const ResourceHandle<Material>& handle) {
        if (!handle.is_valid()) {
            return;
        }

        const auto it = m_materials.resources.find(handle.id);
        if (it == m_materials.resources.end() || it->second.generation != handle.generation) {
            return;
        }

        m_materials.path_to_id.erase(it->second.resource->path());
        m_materials.resources.erase(it);
        m_materials.release_id(handle.id);

        STAR_LOG_DEBUG(LogCategory::Resources, "Destroyed material (id: {})", handle.id);
    }

    void ResourceManager::destroy_all_resources() {
        STAR_LOG_INFO(LogCategory::Resources, "Destroying all resources");

        for (const auto& entry : m_meshes.resources | std::views::values) {
            if (entry.resource->vertex_buffer.is_valid()) {
                m_device.destroy_buffer(entry.resource->vertex_buffer);
            }
            if (entry.resource->index_buffer.is_valid()) {
                m_device.destroy_buffer(entry.resource->index_buffer);
            }
        }
        m_meshes.resources.clear();
        m_meshes.path_to_id.clear();
        m_meshes.free_slots.clear();

        for (const auto& entry : m_textures.resources | std::views::values) {
            if (entry.resource->handle.is_valid()) {
                m_device.destroy_texture(entry.resource->handle);
            }
        }
        m_textures.resources.clear();
        m_textures.path_to_id.clear();
        m_textures.free_slots.clear();

        for (const auto& entry : m_shaders.resources | std::views::values) {
            if (entry.resource->handle.is_valid()) {
                m_device.destroy_shader(entry.resource->handle);
            }
        }
        m_shaders.resources.clear();
        m_shaders.path_to_id.clear();
        m_shaders.free_slots.clear();

        m_materials.resources.clear();
        m_materials.path_to_id.clear();
        m_materials.free_slots.clear();
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

        mesh.bounds = mesh.bounding_box();
    }

    void ResourceManager::upload_texture_to_gpu(Texture& /*texture*/) {
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

        m_default_shader = register_builtin_shader("__simple_shader", BuiltinShader::Material);

        if (!m_default_shader.is_valid()) {
            STAR_LOG_ERROR(LogCategory::Resources, "Failed to create default embedded shader!");
        } else {
            auto default_mat = std::make_unique<Material>();
            default_mat->shader = m_default_shader;
            default_mat->albedo_color = Vector4{1.0f, 1.0f, 1.0f, 1.0f};
            m_default_material = create_material("__default_material", std::move(default_mat));

            STAR_LOG_INFO(LogCategory::Resources, "Default material created with embedded shader");
        }

        STAR_LOG_INFO(LogCategory::Resources, "Default resources created");
    }
} // namespace star::resources
