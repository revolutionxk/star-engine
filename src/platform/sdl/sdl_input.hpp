#pragma once

#include <unordered_set>

#include "star/core/event_dispatcher.hpp"
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

        InputListenerHandle add_key_listener(KeyEventHandler handler, i32 priority = 0) override;
        void remove_key_listener(InputListenerHandle handle) override;

        InputListenerHandle add_mouse_button_listener(MouseButtonEventHandler handler, i32 priority = 0) override;
        void remove_mouse_button_listener(InputListenerHandle handle) override;

        InputListenerHandle add_mouse_move_listener(MouseMoveEventHandler handler) override;
        void remove_mouse_move_listener(InputListenerHandle handle) override;

        InputListenerHandle add_scroll_listener(MouseScrollEventHandler handler, i32 priority = 0) override;
        void remove_scroll_listener(InputListenerHandle handle) override;

        void update() override;

        void process_event(const SDL_Event& event);

      private:
        static KeyCode sdl_keycode_to_keycode(SDL_Keycode sdl_key);
        static MouseButton sdl_button_to_mouse_button(u8 sdl_button);

        std::unordered_set<KeyCode> m_pressed_keys;
        std::unordered_set<KeyCode> m_just_pressed_keys;
        std::unordered_set<KeyCode> m_just_released_keys;

        std::unordered_set<MouseButton> m_pressed_mouse_buttons;
        std::unordered_set<MouseButton> m_just_pressed_mouse_buttons;
        std::unordered_set<MouseButton> m_just_released_mouse_buttons;

        Vector2 m_mouse_position{};
        Vector2 m_mouse_delta{};
        Vector2 m_last_mouse_position{};

        EventDispatcher<KeyEvent> m_key_dispatcher;
        EventDispatcher<MouseButtonEvent> m_mouse_button_dispatcher;
        VoidEventDispatcher<MouseMoveEvent> m_mouse_move_dispatcher;
        EventDispatcher<MouseScrollEvent> m_scroll_dispatcher;
    };

} // namespace star::platform::sdl
