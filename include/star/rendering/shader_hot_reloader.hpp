#pragma once
#include <filesystem>
#include <vector>

#include "star/core/types.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/graphics/shader.hpp"

namespace star::resources {
    class ResourceManager;
}

namespace star::rendering {
    class ShaderHotReloader {
      public:
        explicit ShaderHotReloader(resources::ResourceManager& rm);

        void watch(graphics::ResourceHandle<graphics::Shader> handle);
        void unwatch(graphics::ResourceHandle<graphics::Shader> handle);
        void clear();
        
        void tick(f32 delta_time, f32 poll_interval_seconds = 0.5f);

      private:
        struct Entry {
            graphics::ResourceHandle<graphics::Shader> handle;
            std::filesystem::path vertex_path;
            std::filesystem::path fragment_path;
            std::filesystem::file_time_type vertex_mtime{};
            std::filesystem::file_time_type fragment_mtime{};
        };

        resources::ResourceManager& m_resource_manager;
        std::vector<Entry> m_entries;
        f32 m_time_since_poll = 0.0f;
    };

} // namespace star::rendering
