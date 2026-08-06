#pragma once
#include "star/rendering/renderer.hpp"

namespace star::rendering {
    class DebugRenderer;

    class DebugRenderPass final : public IRenderPass {
        using Super = IRenderPass;

      public:
        explicit DebugRenderPass(DebugRenderer& debug_renderer);

        std::string get_name() const override {
            return "Debug";
        }

        u8 get_priority() const override {
            return 200;
        }

        std::vector<std::string_view> dependencies() const override {
            return {"SceneRenderPass"};
        }

        void pre_render(const FrameContext& frame) override;
        void render(const RenderContext& ctx) override;

      private:
        DebugRenderer& m_debug_renderer;
    };

} // namespace star::rendering
