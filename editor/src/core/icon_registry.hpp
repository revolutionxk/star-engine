#pragma once

#include <string>
#include <unordered_map>

#include "star/core/types.hpp"

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::editor {
    class IconRegistry {
      public:
        explicit IconRegistry(resources::ResourceManager& resources) : m_resources(resources) {}

        [[nodiscard]] u64 icon(const std::string& name);

      private:
        static constexpr int RASTER_SIZE = 128;

        resources::ResourceManager& m_resources;
        std::unordered_map<std::string, u64> m_cache;
    };
} // namespace star::editor
