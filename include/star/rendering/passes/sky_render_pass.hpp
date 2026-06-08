#pragma once

#include "star/core/types.hpp"
#include "star/rendering/procedural_sky.hpp"
#include "star/rendering/renderer.hpp"

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::systems {
    class RenderSystem;
} // namespace star::systems

namespace star::rendering {

    class SkyRenderPass final : public IRenderPass {
        using Super = IRenderPass;

      public:
        SkyRenderPass(systems::RenderSystem& render_system, resources::ResourceManager& resource_manager);
        ~SkyRenderPass() override;

        [[nodiscard]] std::string get_name() const override {
            return "SkyRenderPass";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 150;
        }

        [[nodiscard]] std::vector<std::string_view> dependencies() const override {
            return {"SceneRenderPass"};
        }

        void pre_render(const FrameContext& frame) override;
        void render(const RenderContext& ctx) override;

      private:
        systems::RenderSystem& m_render_system;
        ProceduralSky m_sky;
        f32 m_elapsed_time = 0.0f;
        bool m_has_sky_this_frame = false;
    };
} // namespace star::rendering
