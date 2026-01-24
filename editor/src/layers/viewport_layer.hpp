#pragma once

#include <memory>

#include "editor_window.hpp"
#include "star/application/layer.hpp"

namespace star::graphics {
    class Device;
}

namespace star::scene {
    class SceneManager;
}

namespace star::systems {
    class RenderSystem;
}

namespace star::rendering {
    class RenderTarget;
}

namespace star::editor {
    class ViewportLayer : public application::Layer {
      public:
        ViewportLayer(EditorWindow* editor_window);
        ~ViewportLayer() override = default;

        bool initialize() override;
        void shutdown() override;
        void update(f32 delta_time) override;
        void render() override;

      private:
        void setup_scene_content();
        void setup_render_target();
        void render_scene_to_texture();

        std::unique_ptr<rendering::RenderTarget> m_scene_render_target;
        systems::RenderSystem* m_render_system{nullptr};
        u32 m_viewport_width{1280};
        u32 m_viewport_height{720};
        EditorWindow* m_editor_window{nullptr};
    };
} // namespace star::editor
