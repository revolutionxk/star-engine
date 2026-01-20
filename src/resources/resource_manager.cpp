#include "star/resources/resource_manager.hpp"

#include "star/graphics/buffer.hpp"
#include "star/graphics/mesh.hpp"
#include "star/graphics/texture.hpp"

namespace star::resources {
    ResourceManager::ResourceManager(Device& device) : m_device(device) {
        STAR_LOG_INFO(LogCategory::Resources, "Initializing ResourceManager");

        init_default_resources();
    }

    ResourceHandle<Mesh> ResourceManager::load_mesh(const std::string& path) {
        return {};
    }

    ResourceHandle<Mesh> ResourceManager::create_mesh(const std::string& name, std::unique_ptr<Mesh> mesh) {
        return {};
    }

    Mesh* ResourceManager::get_mesh(const ResourceHandle<Mesh>& handle) {
        return nullptr;
    }

    void ResourceManager::destroy_mesh(const ResourceHandle<Mesh>& handle) {}

    ResourceHandle<Texture> ResourceManager::load_texture(const std::string& path) {
        return {};
    }

    ResourceHandle<Texture> ResourceManager::create_texture(const std::string& name, std::unique_ptr<Texture> texture) {
        return {};
    }

    Texture* ResourceManager::get_texture(const ResourceHandle<Texture>& handle) {
        return nullptr;
    }

    void ResourceManager::destroy_texture(const ResourceHandle<Texture>& handle) {}

    ResourceHandle<Shader> ResourceManager::load_shader(const std::string& vertex_path,
                                                        const std::string& fragment_path) {
        return {};
    }

    void ResourceManager::destroy_all_resources() {}

    void ResourceManager::garbage_collect() {}

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

    void ResourceManager::upload_texture_to_gpu(Texture& texture) {}

    void ResourceManager::init_default_resources() {}
} // namespace star::resources
