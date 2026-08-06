#pragma once

#include <memory>

#include "star/core/types.hpp"

namespace star::rendering {
    struct RenderScene;
}

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::scene {
    class Scene;

    class SceneRenderExtractor {
      public:
        SceneRenderExtractor();
        ~SceneRenderExtractor();

        SceneRenderExtractor(const SceneRenderExtractor&) = delete;
        SceneRenderExtractor& operator=(const SceneRenderExtractor&) = delete;
        SceneRenderExtractor(SceneRenderExtractor&&) noexcept;
        SceneRenderExtractor& operator=(SceneRenderExtractor&&) noexcept;
        
        void set_resource_manager(resources::ResourceManager* resources) noexcept {
            m_resources = resources;
        }

        void extract(Scene& scene, rendering::RenderScene& out, f32 alpha = 1.0f) const;

        void reset();

      private:
        struct QueryCache;
        std::unique_ptr<QueryCache> m_cache;
        resources::ResourceManager* m_resources{nullptr};
    };
} // namespace star::scene
