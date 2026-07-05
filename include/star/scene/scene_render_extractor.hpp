#pragma once

#include <memory>

#include "star/core/types.hpp"

namespace star::rendering {
    struct RenderScene;
}

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

        void extract(Scene& scene, rendering::RenderScene& out) const;

        void reset();

      private:
        struct QueryCache;
        std::unique_ptr<QueryCache> m_cache;
    };
} // namespace star::scene
