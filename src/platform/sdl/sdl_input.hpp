#pragma once

#include <unordered_map>
#include <unordered_set>

#include "star/platform/input/input.hpp"
#include "SDL3/SDL.h"

namespace star::platform::sdl {
    class SDLInput final : public Input {
      public:
        SDLInput();
        ~SDLInput() override;

        [[nodiscard]] bool is_key_pressed(KeyCode key) const override;
        [[nodiscard]] bool is_key_just_pressed(KeyCode key) const override;
        [[nodiscard]] bool is_key_just_released(KeyCode key) const override;

        [[nodiscard]] bool is_mouse_button_pressed(MouseButton button) const override;
        [[nodiscard]] bool is_mouse_button_just_pressed(MouseButton button) const override;
        [[nodiscard]] bool is_mouse_button_just_released(MouseButton button) const override;
        [[nodiscard]] Vector2 mouse_position() const override;
        [[nodiscard]] Vector2 mouse_delta() const override;

        void set_key_event_callback(KeyEventCallback callback) override;
        void set_mouse_button_event_callback(MouseButtonEventCallback callback) override;
        void set_mouse_move_event_callback(MouseMoveEventCallback callback) override;
        void set_mouse_scroll_event_callback(MouseScrollEventCallback callback) override;

        void update() override;

        void process_event(const SDL_Event& event);

      private:
        static KeyCode sdl_keycode_to_keycode(SDL_Keycode sdl_key);
        static [[nodiscard]] MouseButton sdl_button_to_mouse_button(u8 sdl_button);

        std::unordered_set<KeyCode> m_pressed_keys;
        std::unordered_set<KeyCode> m_just_pressed_keys;
        std::unordered_set<KeyCode> m_just_released_keys;

        std::unordered_set<MouseButton> m_pressed_mouse_buttons;
        std::unordered_set<MouseButton> m_just_pressed_mouse_buttons;
        std::unordered_set<MouseButton> m_just_released_mouse_buttons;

        Vector2 m_mouse_position{};
        Vector2 m_mouse_delta{};
        Vector2 m_last_mouse_position{};

        KeyEventCallback m_key_event_callback;
        MouseButtonEventCallback m_mouse_button_event_callback;
        MouseMoveEventCallback m_mouse_move_event_callback;
        MouseScrollEventCallback m_mouse_scroll_event_callback;
    };

} // namespace star::platform::sdl
