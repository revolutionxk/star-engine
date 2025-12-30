#pragma once

#include "star/export.hpp"
#include "star/app/app_component.hpp"
#include <bgfx/bgfx.h>
#include <memory>
#include <imgui.h>

struct ImGuiContext;
struct ImDrawData;

namespace star {
    class App;
    class ImGuiComponentImpl;

    struct ImguiTextureData
    {
        bgfx::TextureHandle handle;
        bool alpha_blend;
        uint8_t mip;

        ImguiTextureData(const bgfx::TextureHandle& handle);

        ImguiTextureData(const ImguiTextureData&) = default;

        ImguiTextureData(ImTextureID id);

        operator ImTextureID() const;

        operator bgfx::TextureHandle() const;

        operator bgfx::TextureHandle& ();
    };

    class STAR_EXPORT IImguiRenderer {
    public:
        virtual ~IImguiRenderer() = default;

        virtual void imgui_setup() {
        }

        virtual void imgui_render() = 0;
    };

    class STAR_EXPORT ImGuiComponent final : public ITypeAppComponent<ImGuiComponent> {
    public:
        explicit ImGuiComponent(IImguiRenderer &renderer, float font_size = 16.0f);

        ~ImGuiComponent() override;

        ImGuiComponent(const ImGuiComponent &) = delete;

        ImGuiComponent &operator=(const ImGuiComponent &) = delete;

        void init(App &app) override;

        void shutdown() override;

        void update(float delta_time) override;

        void render() override;

        bgfx::ViewId render_reset(bgfx::ViewId view_id) override;

        ImGuiContext *get_context() const;

        bool get_input_enabled() const;

        ImGuiComponent &set_input_enabled(bool enabled);

        ImGuiComponent &update_fonts();

    private:
        std::unique_ptr<ImGuiComponentImpl> _impl;
    };
}
