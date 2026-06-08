#pragma once

#include <functional>
#include <vector>

#include <imgui_internal.h>

#include "star/imgui/imgui_platform_backend.hpp"
#include "star/imgui/imgui_renderer.hpp"
#include "star/rendering/renderer.hpp"

namespace star::platform {
    class ImGuiSystem;
}

namespace star::graphics {
    class Device;
    class DeviceContext;
} // namespace star::graphics

namespace star::rendering {
    class ImGuiRenderPass final : public IRenderPass {
        using Super = IRenderPass;

      public:
        ImGuiRenderPass(std::unique_ptr<platform::IImGuiPlatformBackend> platform_backend,
                        std::unique_ptr<platform::IImGuiRenderer> renderer);
        ~ImGuiRenderPass() override;

        [[nodiscard]] std::string get_name() const override {
            return "ImGuiRenderPass";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 255;
        }

        [[nodiscard]] std::vector<std::string_view> dependencies() const override {
            return {"Debug"};
        }

        void pre_render(const FrameContext& frame) override;
        void render(const RenderContext& ctx) override;
        void post_render(const FrameContext& frame) override;
        u32 reset(u32 width, u32 height) override;

        using ImGuiCallback = std::function<void()>;

        void add_imgui_callback(ImGuiCallback callback) {
            m_imgui_callbacks.push_back(std::move(callback));
        }

        void clear_imgui_callbacks() {
            m_imgui_callbacks.clear();
        }

        static void setup_style();

        void set_window(const platform::Window* window);

      private:
        ImGuiContext* m_context = nullptr;
        std::unique_ptr<platform::IImGuiPlatformBackend> m_platform_backend;
        std::unique_ptr<platform::IImGuiRenderer> m_renderer;
        bool m_initialized = false;
        std::vector<ImGuiCallback> m_imgui_callbacks;
    };
} // namespace star::rendering
