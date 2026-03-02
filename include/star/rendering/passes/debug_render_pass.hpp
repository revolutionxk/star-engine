#pragma once
#include "star/rendering/renderer.hpp"

namespace star::rendering {
    class DebugRenderer;
    class Viewport;

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

        void pre_render(f32 delta_time) override;
        void render(graphics::DeviceContext& context, u32 view_id) override;
        void post_render(f32 delta_time) override;
        u32 reset(u32 width = 0, u32 height = 0) override;

        void set_viewport(Viewport* viewport) {
            m_viewport = viewport;
        }

        [[nodiscard]] Viewport* viewport() const {
            return m_viewport;
        }

      private:
        DebugRenderer& m_debug_renderer;
        Viewport* m_viewport{nullptr};
    };

} // namespace star::rendering
