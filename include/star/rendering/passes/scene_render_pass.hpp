#pragma once

#include "star/rendering/renderer.hpp"

namespace star::scene {
    class Scene;
}

namespace star::systems {
    class RenderSystem;
}

namespace star::graphics {
    class Device;
    class DeviceContext;
} // namespace star::graphics

namespace star::rendering {
    class SceneRenderPass final : public IRenderPass {
      public:
        explicit SceneRenderPass(systems::RenderSystem& render_system);
        ~SceneRenderPass() override = default;

        [[nodiscard]] std::string get_name() const override {
            return "SceneRenderPass";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 100;
        }

        void pre_render(f32 delta_time) override;
        void render(graphics::DeviceContext& context, u32 view_id) override;
        void post_render(f32 delta_time) override;
        void reset(u32 width, u32 height) override;

        void set_scene(scene::Scene* scene) {
            m_scene = scene;
        }

        scene::Scene* get_scene() const {
            return m_scene;
        }

      private:
        systems::RenderSystem& m_render_system;
        scene::Scene* m_scene = nullptr;
    };

} // namespace star::rendering
