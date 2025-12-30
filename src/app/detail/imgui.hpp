#pragma once

#include "star/app/imgui_component.hpp"
#include "star/utils/memory/optional_ref.hpp"
#include "star/app/input.hpp"
#include <imgui.h>
#include <unordered_map>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

namespace star
{
    class App;
    class Texture;
    class Program;
    class Input;

    struct ImguiUtils
    {
        static glm::vec2 convert(const ImVec2 &v)
        {
            return {v.x, v.y};
        }

        static ImVec2 convert(const glm::vec2 &v)
        {
            return {v.x, v.y};
        }

        static glm::vec4 convert(const ImVec4 &v)
        {
            return {v.x, v.y, v.z, v.w};
        }

        static ImVec4 convert(const glm::vec4 &v)
        {
            return {v.x, v.y, v.z, v.w};
        }

        static uint16_t convert_uint16(float v)
        {
            v = std::max(v, 0.0f);
            v = std::min(v, 65535.0f);
            return static_cast<uint16_t>(v);
        }
    };

    class ImguiRenderPass
    {
    public:
        ImguiRenderPass(IImguiRenderer &renderer, ImGuiContext *imgui);

        ~ImguiRenderPass();

        bgfx::ViewId render_reset(bgfx::ViewId view_id);

        void render() const;

        void update_fonts();

    private:
        IImguiRenderer &_renderer;
        ImGuiContext *_imgui;
        std::optional<bgfx::ViewId> _view_id;
        bgfx::TextureHandle _fonts_texture;
        bgfx::ProgramHandle _program;
        bgfx::VertexLayout _vertex_layout;
        bgfx::UniformHandle _texture_uniform;

        static void begin_frame();

        bool render(bgfx::Encoder &encoder, ImDrawData *draw_data) const;

        bool end_frame(bgfx::Encoder &encoder) const;
    };

    class ImGuiComponentImpl : public IKeyboardListener
    {
    public:
        explicit ImGuiComponentImpl(IImguiRenderer &renderer, float font_size);

        ~ImGuiComponentImpl();

        void init(App &app);

        void shutdown();

        bgfx::ViewId render_reset(bgfx::ViewId view_id);

        void render() const;

        void update(float dt) const;

        [[nodiscard]] ImGuiContext *get_context() const;

        [[nodiscard]] bool get_input_enabled() const;

        void set_input_enabled(bool enabled);

        void update_fonts();

        // IKeyboardListener implementation
        void on_keyboard_char(const UtfChar &chr) override;

    private:
        using KeyboardMap = std::unordered_map<KeyboardKey, ImGuiKey>;
        using GamepadMap = std::unordered_map<GamepadButton, ImGuiKey>;

        IImguiRenderer &_renderer;
        OptionalRef<App> _app;
        ImGuiContext *_imgui;
        bool _input_enabled;
        float _font_size;
        std::optional<ImguiRenderPass> _render_pass;

        static const KeyboardMap &get_keyboard_map();

        static const GamepadMap &get_gamepad_map();

        void update_input(float dt) const;
    };
}
