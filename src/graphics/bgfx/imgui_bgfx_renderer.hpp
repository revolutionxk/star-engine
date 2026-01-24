#pragma once

#include <bgfx/bgfx.h>

#include "star/platform/imgui_renderer.hpp"

namespace star::graphics {

    class ImGuiBGFXRenderer final : public platform::IImGuiRenderer {
      public:
        ImGuiBGFXRenderer() = default;
        ~ImGuiBGFXRenderer() override;

        ImGuiBGFXRenderer(const ImGuiBGFXRenderer&) = delete;
        ImGuiBGFXRenderer& operator=(const ImGuiBGFXRenderer&) = delete;

        bool initialize() override;
        void shutdown() override;
        void render(u32 view_id) override;
        void reset(u32 width, u32 height) override;

        [[nodiscard]] bool is_initialized() const override {
            return m_initialized;
        }

      private:
        struct Data {
            bgfx::VertexLayout vertex_layout;
            bgfx::ProgramHandle shader_program = BGFX_INVALID_HANDLE;
            bgfx::TextureHandle font_texture = BGFX_INVALID_HANDLE;
            bgfx::UniformHandle s_tex = BGFX_INVALID_HANDLE;
        };

        Data m_data;
        bool m_initialized = false;
    };

} // namespace star::graphics
