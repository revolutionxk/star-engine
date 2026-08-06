#pragma once

#include "star/rendering/renderer.hpp"

namespace star::systems {
    class RenderSystem;
}

namespace star::rendering {

    class SceneRenderPass final : public IRenderPass {
        using Super = IRenderPass;

      public:
        explicit SceneRenderPass(systems::RenderSystem& render_system);
        ~SceneRenderPass() override = default;

        [[nodiscard]] std::string get_name() const override {
            return "SceneRenderPass";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 100;
        }

        void render(const RenderContext& ctx) override;

      private:
        systems::RenderSystem& m_render_system;
    };

} // namespace star::rendering
