#include "star/app/imgui_component.hpp"
#include "detail/imgui.hpp"
#include "star/app/app.hpp"
#include "star/app/window.hpp"
#include "star/app/input.hpp"
#include <imgui.h>
#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <bx/math.h>
#include "star/graphics/shaders.hpp"

namespace star
{
    ImguiTextureData::ImguiTextureData(const bgfx::TextureHandle &handle)
        : handle{handle}, alpha_blend{false}, mip{0}
    {
    }

    ImguiTextureData::ImguiTextureData(const ImTextureID id)
    {
        const union
        {
            ImTextureID id;
            ImguiTextureData data;
        } v = {id};

        *this = v.data;
    }

    ImguiTextureData::operator ImTextureID() const
    {
        const union
        {
            ImguiTextureData data;
            ImTextureID id;
        } v = {*this};

        return v.id;
    }

    ImguiTextureData::operator bgfx::TextureHandle() const
    {
        return handle;
	}

    ImguiTextureData::operator bgfx::TextureHandle& ()
    {
		return handle;
    }
    
    ImguiRenderPass::ImguiRenderPass(IImguiRenderer &renderer, ImGuiContext *imgui)
        : _renderer{renderer},
          _imgui{imgui},
          _fonts_texture{BGFX_INVALID_HANDLE},
          _program{BGFX_INVALID_HANDLE},
          _texture_uniform{BGFX_INVALID_HANDLE}
    {
        _vertex_layout.begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();

        _texture_uniform = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

        const auto vs_handle = bgfx::createEmbeddedShader(&k_imgui_vs, bgfx::getRendererType(), "v_imgui");
        const auto fs_handle = bgfx::createEmbeddedShader(&k_imgui_fs, bgfx::getRendererType(), "f_imgui");
        _program = bgfx::createProgram(vs_handle, fs_handle, true);

        update_fonts();
    }

    ImguiRenderPass::~ImguiRenderPass()
    {
        if (bgfx::isValid(_texture_uniform))
        {
            bgfx::destroy(_texture_uniform);
            _texture_uniform = BGFX_INVALID_HANDLE;
        }
        if (bgfx::isValid(_program))
        {
            bgfx::destroy(_program);
            _program = BGFX_INVALID_HANDLE;
        }
        if (bgfx::isValid(_fonts_texture))
        {
            bgfx::destroy(_fonts_texture);
            _fonts_texture = BGFX_INVALID_HANDLE;
        }
    }

    void ImguiRenderPass::update_fonts()
    {
        unsigned char *data;
        int width, height;
        const ImGuiIO &io = ImGui::GetIO();
        io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);

        if (bgfx::isValid(_fonts_texture))
        {
            bgfx::destroy(_fonts_texture);
        }

        _fonts_texture = bgfx::createTexture2D(
            static_cast<uint16_t>(width),
            static_cast<uint16_t>(height),
            false, 1,
            bgfx::TextureFormat::BGRA8, 0,
            bgfx::copy(data, width * height * 4));

        io.Fonts->SetTexID(_fonts_texture.idx);
    }

    bgfx::ViewId ImguiRenderPass::render_reset(bgfx::ViewId view_id)
    {
        bgfx::setViewName(view_id, "ImGui");
        bgfx::setViewMode(view_id, bgfx::ViewMode::Sequential);
        _view_id = view_id;
        return ++view_id;
    }

    void ImguiRenderPass::render() const
    {
        ImGui::SetCurrentContext(_imgui);
        begin_frame();
        _renderer.imgui_render();
        const auto encoder = bgfx::begin();
        end_frame(*encoder);
        bgfx::end(encoder);
        ImGui::SetCurrentContext(nullptr);
    }

    void ImguiRenderPass::begin_frame()
    {
        ImGui::NewFrame();
    }

    bool ImguiRenderPass::end_frame(bgfx::Encoder &encoder) const
    {
        ImGui::Render();
        return render(encoder, ImGui::GetDrawData());
    }

    bool ImguiRenderPass::render(bgfx::Encoder &encoder, ImDrawData *draw_data) const
    {
        if (!_view_id)
        {
            return false;
        }

        const auto clip_pos = ImguiUtils::convert(draw_data->DisplayPos);
        const auto size = ImguiUtils::convert(draw_data->DisplaySize);
        const auto clip_scale = ImguiUtils::convert(draw_data->FramebufferScale);
        const auto clip_size = size / clip_scale;

        if (clip_size.x <= 0 || clip_size.y <= 0)
        {
            return false;
        }

        const auto view_id = _view_id.value();

        float ortho[16];
        bx::mtxOrtho(ortho,
                     clip_pos.x, clip_pos.x + clip_size.x,
                     clip_pos.y + clip_size.y, clip_pos.y,
                     0.0f, 1000.0f,
                     0.0f, bgfx::getCaps()->homogeneousDepth);
        bgfx::setViewTransform(view_id, nullptr, ortho);
        bgfx::setViewRect(view_id, 0, 0,
                          ImguiUtils::convert_uint16(size.x),
                          ImguiUtils::convert_uint16(size.y));

        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList *cmd_list = draw_data->CmdLists[n];

            const auto num_vertices = static_cast<uint32_t>(cmd_list->VtxBuffer.size());
            const auto num_indices = static_cast<uint32_t>(cmd_list->IdxBuffer.size());

            if (num_vertices != bgfx::getAvailTransientVertexBuffer(num_vertices, _vertex_layout) ||
                num_indices != bgfx::getAvailTransientIndexBuffer(num_indices))
            {
                break;
            }

            bgfx::TransientVertexBuffer tvb{};
            bgfx::TransientIndexBuffer tib{};

            bgfx::allocTransientVertexBuffer(&tvb, num_vertices, _vertex_layout);
            bgfx::allocTransientIndexBuffer(&tib, num_indices);

            auto *verts = reinterpret_cast<ImDrawVert *>(tvb.data);
            memcpy(verts, cmd_list->VtxBuffer.begin(), num_vertices * sizeof(ImDrawVert));

            auto *indices = reinterpret_cast<ImDrawIdx *>(tib.data);
            memcpy(indices, cmd_list->IdxBuffer.begin(), num_indices * sizeof(ImDrawIdx));

            for (const ImDrawCmd *cmd = cmd_list->CmdBuffer.begin(), *cmdEnd = cmd_list->CmdBuffer.end();
                 cmd != cmdEnd; ++cmd)
            {
                if (cmd->UserCallback)
                {
                    cmd->UserCallback(cmd_list, cmd);
                }
                else if (cmd->ElemCount != 0)
                {
                    constexpr uint64_t state = 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA |
                                               BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA,
                                                                     BGFX_STATE_BLEND_INV_SRC_ALPHA);

                    const auto xx = static_cast<uint16_t>(std::max(cmd->ClipRect.x, 0.0f));
                    const auto yy = static_cast<uint16_t>(std::max(cmd->ClipRect.y, 0.0f));
                    encoder.setScissor(
                        xx, yy,
                        static_cast<uint16_t>(std::min(cmd->ClipRect.z, 65535.0f)) - xx,
                        static_cast<uint16_t>(std::min(cmd->ClipRect.w, 65535.0f)) - yy);

                    encoder.setState(state);

                    const bgfx::TextureHandle texture = {
                        static_cast<uint16_t>(cmd->GetTexID() & 0xffff)};
                    encoder.setTexture(0, _texture_uniform, texture);
                    encoder.setVertexBuffer(0, &tvb, 0, num_vertices);
                    encoder.setIndexBuffer(&tib, cmd->IdxOffset, cmd->ElemCount);
                    encoder.submit(view_id, _program);
                }
            }
        }

        return true;
    }

    const ImGuiComponentImpl::KeyboardMap &ImGuiComponentImpl::get_keyboard_map()
    {
        static const KeyboardMap map = {
            {KeyboardKey::Tab, ImGuiKey_Tab},
            {KeyboardKey::Left, ImGuiKey_LeftArrow},
            {KeyboardKey::Right, ImGuiKey_RightArrow},
            {KeyboardKey::Up, ImGuiKey_UpArrow},
            {KeyboardKey::Down, ImGuiKey_DownArrow},
            {KeyboardKey::PageUp, ImGuiKey_PageUp},
            {KeyboardKey::PageDown, ImGuiKey_PageDown},
            {KeyboardKey::Home, ImGuiKey_Home},
            {KeyboardKey::End, ImGuiKey_End},
            {KeyboardKey::Insert, ImGuiKey_Insert},
            {KeyboardKey::Delete, ImGuiKey_Delete},
            {KeyboardKey::Backspace, ImGuiKey_Backspace},
            {KeyboardKey::Space, ImGuiKey_Space},
            {KeyboardKey::Enter, ImGuiKey_Enter},
            {KeyboardKey::Escape, ImGuiKey_Escape},
            {KeyboardKey::LeftCtrl, ImGuiKey_LeftCtrl},
            {KeyboardKey::LeftShift, ImGuiKey_LeftShift},
            {KeyboardKey::LeftAlt, ImGuiKey_LeftAlt},
            {KeyboardKey::LeftSuper, ImGuiKey_LeftSuper},
            {KeyboardKey::RightCtrl, ImGuiKey_RightCtrl},
            {KeyboardKey::RightShift, ImGuiKey_RightShift},
            {KeyboardKey::RightAlt, ImGuiKey_RightAlt},
            {KeyboardKey::RightSuper, ImGuiKey_RightSuper},
            {KeyboardKey::A, ImGuiKey_A},
            {KeyboardKey::B, ImGuiKey_B},
            {KeyboardKey::C, ImGuiKey_C},
            {KeyboardKey::D, ImGuiKey_D},
            {KeyboardKey::E, ImGuiKey_E},
            {KeyboardKey::F, ImGuiKey_F},
            {KeyboardKey::G, ImGuiKey_G},
            {KeyboardKey::H, ImGuiKey_H},
            {KeyboardKey::I, ImGuiKey_I},
            {KeyboardKey::J, ImGuiKey_J},
            {KeyboardKey::K, ImGuiKey_K},
            {KeyboardKey::L, ImGuiKey_L},
            {KeyboardKey::M, ImGuiKey_M},
            {KeyboardKey::N, ImGuiKey_N},
            {KeyboardKey::O, ImGuiKey_O},
            {KeyboardKey::P, ImGuiKey_P},
            {KeyboardKey::Q, ImGuiKey_Q},
            {KeyboardKey::R, ImGuiKey_R},
            {KeyboardKey::S, ImGuiKey_S},
            {KeyboardKey::T, ImGuiKey_T},
            {KeyboardKey::U, ImGuiKey_U},
            {KeyboardKey::V, ImGuiKey_V},
            {KeyboardKey::W, ImGuiKey_W},
            {KeyboardKey::X, ImGuiKey_X},
            {KeyboardKey::Y, ImGuiKey_Y},
            {KeyboardKey::Z, ImGuiKey_Z},
            {KeyboardKey::Num0, ImGuiKey_0},
            {KeyboardKey::Num1, ImGuiKey_1},
            {KeyboardKey::Num2, ImGuiKey_2},
            {KeyboardKey::Num3, ImGuiKey_3},
            {KeyboardKey::Num4, ImGuiKey_4},
            {KeyboardKey::Num5, ImGuiKey_5},
            {KeyboardKey::Num6, ImGuiKey_6},
            {KeyboardKey::Num7, ImGuiKey_7},
            {KeyboardKey::Num8, ImGuiKey_8},
            {KeyboardKey::Num9, ImGuiKey_9},
            {KeyboardKey::F1, ImGuiKey_F1},
            {KeyboardKey::F2, ImGuiKey_F2},
            {KeyboardKey::F3, ImGuiKey_F3},
            {KeyboardKey::F4, ImGuiKey_F4},
            {KeyboardKey::F5, ImGuiKey_F5},
            {KeyboardKey::F6, ImGuiKey_F6},
            {KeyboardKey::F7, ImGuiKey_F7},
            {KeyboardKey::F8, ImGuiKey_F8},
            {KeyboardKey::F9, ImGuiKey_F9},
            {KeyboardKey::F10, ImGuiKey_F10},
            {KeyboardKey::F11, ImGuiKey_F11},
            {KeyboardKey::F12, ImGuiKey_F12},
            {KeyboardKey::Apostrophe, ImGuiKey_Apostrophe},
            {KeyboardKey::Comma, ImGuiKey_Comma},
            {KeyboardKey::Minus, ImGuiKey_Minus},
            {KeyboardKey::Period, ImGuiKey_Period},
            {KeyboardKey::Slash, ImGuiKey_Slash},
            {KeyboardKey::Semicolon, ImGuiKey_Semicolon},
            {KeyboardKey::Equal, ImGuiKey_Equal},
            {KeyboardKey::LeftBracket, ImGuiKey_LeftBracket},
            {KeyboardKey::Backslash, ImGuiKey_Backslash},
            {KeyboardKey::RightBracket, ImGuiKey_RightBracket},
            {KeyboardKey::GraveAccent, ImGuiKey_GraveAccent},
            {KeyboardKey::Keypad0, ImGuiKey_Keypad0},
            {KeyboardKey::Keypad1, ImGuiKey_Keypad1},
            {KeyboardKey::Keypad2, ImGuiKey_Keypad2},
            {KeyboardKey::Keypad3, ImGuiKey_Keypad3},
            {KeyboardKey::Keypad4, ImGuiKey_Keypad4},
            {KeyboardKey::Keypad5, ImGuiKey_Keypad5},
            {KeyboardKey::Keypad6, ImGuiKey_Keypad6},
            {KeyboardKey::Keypad7, ImGuiKey_Keypad7},
            {KeyboardKey::Keypad8, ImGuiKey_Keypad8},
            {KeyboardKey::Keypad9, ImGuiKey_Keypad9},
            {KeyboardKey::KeypadDecimal, ImGuiKey_KeypadDecimal},
            {KeyboardKey::KeypadDivide, ImGuiKey_KeypadDivide},
            {KeyboardKey::KeypadMultiply, ImGuiKey_KeypadMultiply},
            {KeyboardKey::KeypadSubtract, ImGuiKey_KeypadSubtract},
            {KeyboardKey::KeypadAdd, ImGuiKey_KeypadAdd},
            {KeyboardKey::KeypadEnter, ImGuiKey_KeypadEnter},
            {KeyboardKey::KeypadEqual, ImGuiKey_KeypadEqual},
            {KeyboardKey::CapsLock, ImGuiKey_CapsLock},
            {KeyboardKey::ScrollLock, ImGuiKey_ScrollLock},
            {KeyboardKey::Pause, ImGuiKey_Pause},
            {KeyboardKey::PrintScreen, ImGuiKey_PrintScreen},
            {KeyboardKey::Menu, ImGuiKey_Menu},
        };
        return map;
    }

    const ImGuiComponentImpl::GamepadMap &ImGuiComponentImpl::get_gamepad_map()
    {
        static const GamepadMap map = {
            {GamepadButton::A, ImGuiKey_GamepadFaceDown},
            {GamepadButton::B, ImGuiKey_GamepadFaceRight},
            {GamepadButton::X, ImGuiKey_GamepadFaceLeft},
            {GamepadButton::Y, ImGuiKey_GamepadFaceUp},
            {GamepadButton::DPadUp, ImGuiKey_GamepadDpadUp},
            {GamepadButton::DPadDown, ImGuiKey_GamepadDpadDown},
            {GamepadButton::DPadLeft, ImGuiKey_GamepadDpadLeft},
            {GamepadButton::DPadRight, ImGuiKey_GamepadDpadRight},
            {GamepadButton::LeftShoulder, ImGuiKey_GamepadL1},
            {GamepadButton::RightShoulder, ImGuiKey_GamepadR1},
            {GamepadButton::LeftStick, ImGuiKey_GamepadL3},
            {GamepadButton::RightStick, ImGuiKey_GamepadR3},
            {GamepadButton::Back, ImGuiKey_GamepadBack},
            {GamepadButton::Start, ImGuiKey_GamepadStart},
        };
        return map;
    }

    ImGuiComponentImpl::ImGuiComponentImpl(IImguiRenderer &renderer, const float font_size)
        : _renderer{renderer},
          _imgui{nullptr},
          _input_enabled{true},
          _font_size{font_size}
    {
        IMGUI_CHECKVERSION();
    }

    ImGuiComponentImpl::~ImGuiComponentImpl()
    {
        shutdown();
    }

    void ImGuiComponentImpl::init(App &app)
    {
        _app = app;

        _imgui = ImGui::CreateContext();
        ImGui::SetCurrentContext(_imgui);

        ImGuiIO &io = ImGui::GetIO();
        io.DeltaTime = 1.0f / 60.0f;
        io.IniFilename = nullptr;

        ImGuiStyle &style = ImGui::GetStyle();
        ImGui::StyleColorsDark(&style);
        style.FrameRounding = 4.0f;
        style.WindowBorderSize = 0.0f;

        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
        io.ConfigFlags |= 0 | ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_NavEnableKeyboard;

        _renderer.imgui_setup();

        _render_pass.emplace(_renderer, _imgui);

        // Register keyboard listener for text input
        _app->get_input().get_keyboard().add_listener(*this);

        ImGui::SetCurrentContext(nullptr);
    }

    void ImGuiComponentImpl::shutdown()
    {
        if (_app)
        {
            _app->get_input().get_keyboard().remove_listener(*this);
        }

        _app.reset();
        _render_pass.reset();

        if (_imgui != nullptr)
        {
            ImGui::DestroyContext(_imgui);
            _imgui = nullptr;
        }
    }

    bgfx::ViewId ImGuiComponentImpl::render_reset(bgfx::ViewId view_id)
    {
        if (_render_pass)
        {
            return _render_pass->render_reset(view_id);
        }
        return view_id;
    }

    void ImGuiComponentImpl::render() const
    {
        if (_render_pass)
        {
            _render_pass->render();
        }
    }

    void ImGuiComponentImpl::update(const float dt) const
    {
        ImGui::SetCurrentContext(_imgui);
        update_input(dt);
        ImGui::SetCurrentContext(nullptr);
    }

    ImGuiContext *ImGuiComponentImpl::get_context() const
    {
        return _imgui;
    }

    bool ImGuiComponentImpl::get_input_enabled() const
    {
        return _input_enabled;
    }

    void ImGuiComponentImpl::set_input_enabled(const bool enabled)
    {
        _input_enabled = enabled;
    }

    void ImGuiComponentImpl::update_fonts()
    {
        if (_render_pass)
        {
            _render_pass->update_fonts();
        }
    }

    void ImGuiComponentImpl::update_input(const float dt) const
    {
        if (!_input_enabled || !_app)
        {
            return;
        }

        auto &input = _app->get_input();
        ImGuiIO &io = ImGui::GetIO();
        io.DeltaTime = dt;

        const auto &window = _app->get_window();
        const auto size = window.get_size();
        io.DisplaySize = ImguiUtils::convert(glm::vec2(size));
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

        const auto &mouse = input.get_mouse();
        const auto pos = mouse.get_position();
        io.AddMousePosEvent(pos.x, pos.y);

        io.AddMouseButtonEvent(ImGuiMouseButton_Left, mouse.is_button_down(MouseButton::Left));
        io.AddMouseButtonEvent(ImGuiMouseButton_Right, mouse.is_button_down(MouseButton::Right));
        io.AddMouseButtonEvent(ImGuiMouseButton_Middle, mouse.is_button_down(MouseButton::Middle));

        const auto scroll = mouse.get_scroll();
        io.AddMouseWheelEvent(scroll.x, scroll.y);

        const auto &keyboard = input.get_keyboard();

        for (const auto &key_map = get_keyboard_map(); const auto &[key, imgui_key] : key_map)
        {
            io.AddKeyEvent(imgui_key, keyboard.is_key_down(key));
        }

        const auto &controller = input.get_controller();
        if (controller.get_controller_count() > 0)
        {
            const int controller_id = 0;

            for (const auto &gamepad_map = get_gamepad_map(); const auto &[button, imgui_key] : gamepad_map)
            {
                io.AddKeyEvent(imgui_key, controller.is_button_down(controller_id, static_cast<int>(button)));
            }

            const float left_x = controller.get_axis_value(controller_id, static_cast<int>(GamepadAxis::LeftX));
            const float left_y = controller.get_axis_value(controller_id, static_cast<int>(GamepadAxis::LeftY));
            const float right_x = controller.get_axis_value(controller_id, static_cast<int>(GamepadAxis::RightX));
            const float right_y = controller.get_axis_value(controller_id, static_cast<int>(GamepadAxis::RightY));

            io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickLeft, left_x < -0.1f, left_x < 0.0f ? -left_x : 0.0f);
            io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickRight, left_x > 0.1f, left_x > 0.0f ? left_x : 0.0f);
            io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickUp, left_y < -0.1f, left_y < 0.0f ? -left_y : 0.0f);
            io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickDown, left_y > 0.1f, left_y > 0.0f ? left_y : 0.0f);

            io.AddKeyAnalogEvent(ImGuiKey_GamepadRStickLeft, right_x < -0.1f, right_x < 0.0f ? -right_x : 0.0f);
            io.AddKeyAnalogEvent(ImGuiKey_GamepadRStickRight, right_x > 0.1f, right_x > 0.0f ? right_x : 0.0f);
            io.AddKeyAnalogEvent(ImGuiKey_GamepadRStickUp, right_y < -0.1f, right_y < 0.0f ? -right_y : 0.0f);
            io.AddKeyAnalogEvent(ImGuiKey_GamepadRStickDown, right_y > 0.1f, right_y > 0.0f ? right_y : 0.0f);

            const float left_trigger = controller.get_axis_value(controller_id,
                                                                 static_cast<int>(GamepadAxis::LeftTrigger));
            const float right_trigger = controller.get_axis_value(controller_id,
                                                                  static_cast<int>(GamepadAxis::RightTrigger));

            io.AddKeyAnalogEvent(ImGuiKey_GamepadL2, left_trigger > 0.1f, left_trigger);
            io.AddKeyAnalogEvent(ImGuiKey_GamepadR2, right_trigger > 0.1f, right_trigger);
        }
    }

    void ImGuiComponentImpl::on_keyboard_char(const UtfChar &chr)
    {
        if (!_input_enabled || !_imgui)
        {
            return;
        }

        ImGui::SetCurrentContext(_imgui);
        ImGuiIO &io = ImGui::GetIO();
        io.AddInputCharacter(chr);
        ImGui::SetCurrentContext(nullptr);
    }

    ImGuiComponent::ImGuiComponent(IImguiRenderer &renderer, const float font_size)
        : _impl{std::make_unique<ImGuiComponentImpl>(renderer, font_size)}
    {
    }

    ImGuiComponent::~ImGuiComponent() = default;

    void ImGuiComponent::init(App &app)
    {
        _impl->init(app);
    }

    void ImGuiComponent::shutdown()
    {
        _impl->shutdown();
    }

    bgfx::ViewId ImGuiComponent::render_reset(const bgfx::ViewId view_id)
    {
        return _impl->render_reset(view_id);
    }

    void ImGuiComponent::render()
    {
        _impl->render();
    }

    void ImGuiComponent::update(const float dt)
    {
        _impl->update(dt);
    }

    ImGuiContext *ImGuiComponent::get_context() const
    {
        return _impl->get_context();
    }

    bool ImGuiComponent::get_input_enabled() const
    {
        return _impl->get_input_enabled();
    }

    ImGuiComponent &ImGuiComponent::set_input_enabled(const bool enabled)
    {
        _impl->set_input_enabled(enabled);
        return *this;
    }

    ImGuiComponent &ImGuiComponent::update_fonts()
    {
        _impl->update_fonts();
        return *this;
    }
}
