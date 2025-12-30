#include "viewport_panel.hpp"
#include "editor_context.hpp"
#include <imgui.h>

namespace star::editor
{
    ViewportPanel::ViewportPanel(EditorContext &context)
        : EditorPanel("Viewport", context), _view_id(0)
    {
        _framebuffer = BGFX_INVALID_HANDLE;
    }

    ViewportPanel::~ViewportPanel()
    {
        if (bgfx::isValid(_framebuffer))
        {
            bgfx::destroy(_framebuffer);
        }
        if (bgfx::isValid(_render_texture))
        {
            bgfx::destroy(_render_texture);
        }
    }

    void ViewportPanel::on_imgui_render()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport");

        const auto viewport_min = ImGui::GetWindowContentRegionMin();
        const auto viewport_max = ImGui::GetWindowContentRegionMax();

        const auto viewport_size = ImVec2(
            viewport_max.x - viewport_min.x,
            viewport_max.y - viewport_min.y);

        get_context().set_viewport_hovered(ImGui::IsWindowHovered());
        get_context().set_viewport_focused(ImGui::IsWindowFocused());

        if (viewport_size.x > 0 && viewport_size.y > 0)
        {
            const auto new_width = static_cast<uint32_t>(viewport_size.x);
            const auto new_height = static_cast<uint32_t>(viewport_size.y);

            if (!_texture_initialized || new_width != _viewport_width || new_height != _viewport_height)
            {
                _viewport_width = new_width;
                _viewport_height = new_height;

                if (bgfx::isValid(_framebuffer))
                {
                    bgfx::destroy(_framebuffer);
                }
                if (bgfx::isValid(_render_texture))
                {
                    bgfx::destroy(_render_texture);
                }

                _render_texture = bgfx::createTexture2D(
                    static_cast<uint16_t>(_viewport_width),
                    static_cast<uint16_t>(_viewport_height),
                    false,
                    1,
                    bgfx::TextureFormat::RGBA8,
                    BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT);

                _framebuffer = bgfx::createFrameBuffer(1, &_render_texture.handle, true);

                _texture_initialized = true;
            }

            if (bgfx::isValid(_framebuffer))
            {
                bgfx::setViewFrameBuffer(_view_id, _framebuffer);
            }

            if (bgfx::isValid(_render_texture))
            {
                ImGui::Image(static_cast<ImTextureID>(_render_texture), viewport_size);
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }
}
