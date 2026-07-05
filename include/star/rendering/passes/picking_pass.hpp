#pragma once

#include <optional>
#include <vector>

#include "star/core/types.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/rendering/render_target.hpp"
#include "star/rendering/renderer.hpp"

namespace star::graphics {
    class Device;
    struct Shader;
    struct Texture;
} // namespace star::graphics

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::rendering {
    class PickingPass final : public IRenderPass {
      public:
        PickingPass(graphics::Device& device, resources::ResourceManager& resources);
        ~PickingPass() override;

        [[nodiscard]] std::string get_name() const override {
            return "PickingPass";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 240;
        }

        [[nodiscard]] PassScope scope() const override {
            return PassScope::Global;
        }

        [[nodiscard]] bool is_enabled() const override {
            return m_request_pending;
        }

        void render(const RenderContext& ctx) override;

        void request(u32 pixel_x, u32 pixel_y);
        [[nodiscard]] std::optional<u64> poll();

      private:
        static Vector4 encode_id(u32 id);

        graphics::Device& m_device;
        resources::ResourceManager& m_resources;
        graphics::ResourceHandle<graphics::Shader> m_pick_shader;
        RenderTarget m_target;
        graphics::ResourceHandle<graphics::Texture> m_readback;

        bool m_request_pending = false;
        u32 m_pixel_x = 0;
        u32 m_pixel_y = 0;

        bool m_awaiting_readback = false;
        u32 m_ready_frame = 0;
        u8 m_readback_pixel[4] = {0, 0, 0, 0};
        std::vector<u64> m_ids;
    };
} // namespace star::rendering
