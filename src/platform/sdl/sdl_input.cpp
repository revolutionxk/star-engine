#include "sdl_input.hpp"

#include "star/core/logger.hpp"

namespace star::platform::sdl {
    SDLInput::SDLInput() {
        STAR_LOG_INFO(LogCategory::Platform, "SDL Input Manager initialized");
    }

    SDLInput::~SDLInput() {
        STAR_LOG_INFO(LogCategory::Platform, "SDL Input Manager destroyed");
    }

    bool SDLInput::is_key_pressed(const KeyCode key) const {
        return m_pressed_keys.contains(key);
    }

    bool SDLInput::is_key_just_pressed(const KeyCode key) const {
        return m_just_pressed_keys.contains(key);
    }

    bool SDLInput::is_key_just_released(const KeyCode key) const {
        return m_just_released_keys.contains(key);
    }

    bool SDLInput::is_mouse_button_pressed(const MouseButton button) const {
        return m_pressed_mouse_buttons.contains(button);
    }

    bool SDLInput::is_mouse_button_just_pressed(const MouseButton button) const {
        return m_just_pressed_mouse_buttons.contains(button);
    }

    bool SDLInput::is_mouse_button_just_released(const MouseButton button) const {
        return m_just_released_mouse_buttons.contains(button);
    }

    Vector2 SDLInput::mouse_position() const {
        return m_mouse_position;
    }

    Vector2 SDLInput::mouse_delta() const {
        return m_mouse_delta;
    }

    SDLInput::InputListenerHandle SDLInput::add_key_listener(KeyEventHandler handler, const i32 priority) {
        return m_key_dispatcher.add(std::move(handler), priority);
    }

    void SDLInput::remove_key_listener(const InputListenerHandle handle) {
        m_key_dispatcher.remove(handle);
    }

    SDLInput::InputListenerHandle SDLInput::add_mouse_button_listener(MouseButtonEventHandler handler, const i32 priority) {
        return m_mouse_button_dispatcher.add(std::move(handler), priority);
    }

    void SDLInput::remove_mouse_button_listener(const InputListenerHandle handle) {
        m_mouse_button_dispatcher.remove(handle);
    }

    SDLInput::InputListenerHandle SDLInput::add_mouse_move_listener(MouseMoveEventHandler handler) {
        return m_mouse_move_dispatcher.add(std::move(handler));
    }

    void SDLInput::remove_mouse_move_listener(const InputListenerHandle handle) {
        m_mouse_move_dispatcher.remove(handle);
    }

    SDLInput::InputListenerHandle SDLInput::add_scroll_listener(MouseScrollEventHandler handler, const i32 priority) {
        return m_scroll_dispatcher.add(std::move(handler), priority);
    }

    void SDLInput::remove_scroll_listener(const InputListenerHandle handle) {
        m_scroll_dispatcher.remove(handle);
    }

    void SDLInput::update() {
        m_just_pressed_keys.clear();
        m_just_released_keys.clear();
        m_just_pressed_mouse_buttons.clear();
        m_just_released_mouse_buttons.clear();

        m_mouse_delta = m_mouse_position - m_last_mouse_position;
        m_last_mouse_position = m_mouse_position;
    }

    void SDLInput::process_event(const SDL_Event& event) {
        switch (event.type) {
            case SDL_EVENT_KEY_DOWN: {
                if (const KeyCode key = sdl_keycode_to_keycode(event.key.key); key != KeyCode::Unknown) {
                    if (!m_pressed_keys.contains(key)) {
                        m_pressed_keys.insert(key);
                        m_just_pressed_keys.insert(key);

                        KeyEvent key_event{};
                        key_event.key = key;
                        key_event.action = event.key.repeat ? InputAction::Repeat : InputAction::Press;
                        key_event.ctrl_pressed = (event.key.mod & SDL_KMOD_CTRL) != 0;
                        key_event.alt_pressed = (event.key.mod & SDL_KMOD_ALT) != 0;
                        key_event.shift_pressed = (event.key.mod & SDL_KMOD_SHIFT) != 0;
                        m_key_dispatcher.dispatch(key_event);
                    }
                }
                break;
            }

            case SDL_EVENT_KEY_UP: {
                if (const KeyCode key = sdl_keycode_to_keycode(event.key.key); key != KeyCode::Unknown) {
                    if (m_pressed_keys.contains(key)) {
                        m_pressed_keys.erase(key);
                        m_just_released_keys.insert(key);

                        KeyEvent key_event{};
                        key_event.key = key;
                        key_event.action = InputAction::Release;
                        key_event.ctrl_pressed = (event.key.mod & SDL_KMOD_CTRL) != 0;
                        key_event.alt_pressed = (event.key.mod & SDL_KMOD_ALT) != 0;
                        key_event.shift_pressed = (event.key.mod & SDL_KMOD_SHIFT) != 0;
                        m_key_dispatcher.dispatch(key_event);
                    }
                }
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                if (const MouseButton button = sdl_button_to_mouse_button(event.button.button);
                    button != MouseButton::Unknown) {
                    if (!m_pressed_mouse_buttons.contains(button)) {
                        m_pressed_mouse_buttons.insert(button);
                        m_just_pressed_mouse_buttons.insert(button);

                        MouseButtonEvent mouse_event{};
                        mouse_event.button = button;
                        mouse_event.action = InputAction::Press;
                        mouse_event.position = {static_cast<f32>(event.button.x), static_cast<f32>(event.button.y)};
                        m_mouse_button_dispatcher.dispatch(mouse_event);
                    }
                }
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_UP: {
                if (const MouseButton button = sdl_button_to_mouse_button(event.button.button);
                    button != MouseButton::Unknown) {
                    if (m_pressed_mouse_buttons.contains(button)) {
                        m_pressed_mouse_buttons.erase(button);
                        m_just_released_mouse_buttons.insert(button);

                        MouseButtonEvent mouse_event{};
                        mouse_event.button = button;
                        mouse_event.action = InputAction::Release;
                        mouse_event.position = {static_cast<f32>(event.button.x), static_cast<f32>(event.button.y)};
                        m_mouse_button_dispatcher.dispatch(mouse_event);
                    }
                }
                break;
            }

            case SDL_EVENT_MOUSE_MOTION: {
                m_mouse_position = {static_cast<f32>(event.motion.x), static_cast<f32>(event.motion.y)};

                MouseMoveEvent move_event{};
                move_event.position = m_mouse_position;
                move_event.delta = {static_cast<f32>(event.motion.xrel), static_cast<f32>(event.motion.yrel)};
                m_mouse_move_dispatcher.dispatch(move_event);
                break;
            }

            case SDL_EVENT_MOUSE_WHEEL: {
                MouseScrollEvent scroll_event{};
                scroll_event.offset = {static_cast<f32>(event.wheel.x), static_cast<f32>(event.wheel.y)};
                scroll_event.position = m_mouse_position;
                m_scroll_dispatcher.dispatch(scroll_event);
                break;
            }

            default:
                break;
        }
    }

    KeyCode SDLInput::sdl_keycode_to_keycode(const SDL_Keycode sdl_key) {
        switch (sdl_key) {
            case SDLK_A:
                return KeyCode::A;
            case SDLK_B:
                return KeyCode::B;
            case SDLK_C:
                return KeyCode::C;
            case SDLK_D:
                return KeyCode::D;
            case SDLK_E:
                return KeyCode::E;
            case SDLK_F:
                return KeyCode::F;
            case SDLK_G:
                return KeyCode::G;
            case SDLK_H:
                return KeyCode::H;
            case SDLK_I:
                return KeyCode::I;
            case SDLK_J:
                return KeyCode::J;
            case SDLK_K:
                return KeyCode::K;
            case SDLK_L:
                return KeyCode::L;
            case SDLK_M:
                return KeyCode::M;
            case SDLK_N:
                return KeyCode::N;
            case SDLK_O:
                return KeyCode::O;
            case SDLK_P:
                return KeyCode::P;
            case SDLK_Q:
                return KeyCode::Q;
            case SDLK_R:
                return KeyCode::R;
            case SDLK_S:
                return KeyCode::S;
            case SDLK_T:
                return KeyCode::T;
            case SDLK_U:
                return KeyCode::U;
            case SDLK_V:
                return KeyCode::V;
            case SDLK_W:
                return KeyCode::W;
            case SDLK_X:
                return KeyCode::X;
            case SDLK_Y:
                return KeyCode::Y;
            case SDLK_Z:
                return KeyCode::Z;
            case SDLK_0:
                return KeyCode::Key0;
            case SDLK_1:
                return KeyCode::Key1;
            case SDLK_2:
                return KeyCode::Key2;
            case SDLK_3:
                return KeyCode::Key3;
            case SDLK_4:
                return KeyCode::Key4;
            case SDLK_5:
                return KeyCode::Key5;
            case SDLK_6:
                return KeyCode::Key6;
            case SDLK_7:
                return KeyCode::Key7;
            case SDLK_8:
                return KeyCode::Key8;
            case SDLK_9:
                return KeyCode::Key9;
            case SDLK_F1:
                return KeyCode::F1;
            case SDLK_F2:
                return KeyCode::F2;
            case SDLK_F3:
                return KeyCode::F3;
            case SDLK_F4:
                return KeyCode::F4;
            case SDLK_F5:
                return KeyCode::F5;
            case SDLK_F6:
                return KeyCode::F6;
            case SDLK_F7:
                return KeyCode::F7;
            case SDLK_F8:
                return KeyCode::F8;
            case SDLK_F9:
                return KeyCode::F9;
            case SDLK_F10:
                return KeyCode::F10;
            case SDLK_F11:
                return KeyCode::F11;
            case SDLK_F12:
                return KeyCode::F12;
            case SDLK_UP:
                return KeyCode::Up;
            case SDLK_DOWN:
                return KeyCode::Down;
            case SDLK_LEFT:
                return KeyCode::Left;
            case SDLK_RIGHT:
                return KeyCode::Right;
            case SDLK_SPACE:
                return KeyCode::Space;
            case SDLK_RETURN:
                return KeyCode::Enter;
            case SDLK_TAB:
                return KeyCode::Tab;
            case SDLK_ESCAPE:
                return KeyCode::Escape;
            case SDLK_BACKSPACE:
                return KeyCode::Backspace;
            case SDLK_DELETE:
                return KeyCode::Delete;
            case SDLK_LSHIFT:
                return KeyCode::LeftShift;
            case SDLK_RSHIFT:
                return KeyCode::RightShift;
            case SDLK_LCTRL:
                return KeyCode::LeftControl;
            case SDLK_RCTRL:
                return KeyCode::RightControl;
            case SDLK_LALT:
                return KeyCode::LeftAlt;
            case SDLK_RALT:
                return KeyCode::RightAlt;
            case SDLK_CAPSLOCK:
                return KeyCode::CapsLock;
            case SDLK_NUMLOCKCLEAR:
                return KeyCode::NumLock;
            case SDLK_SCROLLLOCK:
                return KeyCode::ScrollLock;
            case SDLK_INSERT:
                return KeyCode::Insert;
            case SDLK_HOME:
                return KeyCode::Home;
            case SDLK_END:
                return KeyCode::End;
            case SDLK_PAGEUP:
                return KeyCode::PageUp;
            case SDLK_PAGEDOWN:
                return KeyCode::PageDown;

            default:
                return KeyCode::Unknown;
        }
    }

    MouseButton SDLInput::sdl_button_to_mouse_button(const u8 sdl_button) {
        switch (sdl_button) {
            case SDL_BUTTON_LEFT:
                return MouseButton::Left;
            case SDL_BUTTON_MIDDLE:
                return MouseButton::Middle;
            case SDL_BUTTON_RIGHT:
                return MouseButton::Right;
            case SDL_BUTTON_X1:
                return MouseButton::X1;
            case SDL_BUTTON_X2:
                return MouseButton::X2;
            default:
                return MouseButton::Unknown;
        }
    }

} // namespace star::platform::sdl
