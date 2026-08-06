#pragma once

#include "star/core/common.hpp"
#include "star/core/event_dispatcher.hpp"
#include "star/math/vector2.hpp"

namespace star::platform {
    enum class KeyCode : u16 {
        A = 'A',
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        Key0 = '0',
        Key1,
        Key2,
        Key3,
        Key4,
        Key5,
        Key6,
        Key7,
        Key8,
        Key9,

        F1 = 0x0100,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,

        Up = 0x0200,
        Down,
        Left,
        Right,

        Space = ' ',
        Enter = '\n',
        Tab = '\t',
        Escape = 0x001B,
        Backspace = '\b',
        Delete = 0x007F,

        LeftShift = 0x0300,
        RightShift,
        LeftControl,
        RightControl,
        LeftAlt,
        RightAlt,

        CapsLock = 0x0400,
        NumLock,
        ScrollLock,
        Insert,
        Home,
        End,
        PageUp,
        PageDown,

        Unknown = 0
    };

    enum class MouseButton : u8 {
        Left = 0,
        Middle,
        Right,
        X1,
        X2,
        Unknown
    };

    enum class InputAction : u8 {
        Press,
        Release,
        Repeat
    };

    struct KeyEvent {
        KeyCode key;
        InputAction action;
        bool ctrl_pressed;
        bool alt_pressed;
        bool shift_pressed;
    };

    struct MouseButtonEvent {
        MouseButton button;
        InputAction action;
        Vector2 position;
    };

    struct MouseMoveEvent {
        Vector2 position;
        Vector2 delta;
    };

    struct MouseScrollEvent {
        Vector2 offset;
        Vector2 position;
    };

    class Input {
      public:
        virtual ~Input() = default;

        [[nodiscard]] virtual bool is_key_pressed(KeyCode key) const = 0;
        [[nodiscard]] virtual bool is_key_just_pressed(KeyCode key) const = 0;
        [[nodiscard]] virtual bool is_key_just_released(KeyCode key) const = 0;

        [[nodiscard]] virtual bool is_mouse_button_pressed(MouseButton button) const = 0;
        [[nodiscard]] virtual bool is_mouse_button_just_pressed(MouseButton button) const = 0;
        [[nodiscard]] virtual bool is_mouse_button_just_released(MouseButton button) const = 0;
        [[nodiscard]] virtual Vector2 mouse_position() const = 0;
        [[nodiscard]] virtual Vector2 mouse_delta() const = 0;

        using KeyEventHandler = std::function<bool(const KeyEvent&)>;
        using MouseButtonEventHandler = std::function<bool(const MouseButtonEvent&)>;
        using MouseMoveEventHandler = std::function<void(const MouseMoveEvent&)>;
        using MouseScrollEventHandler = std::function<bool(const MouseScrollEvent&)>;
        using InputListenerHandle = EventListenerHandle;

        virtual InputListenerHandle add_key_listener(KeyEventHandler handler, i32 priority = 0) = 0;
        virtual void remove_key_listener(InputListenerHandle handle) = 0;

        virtual InputListenerHandle add_mouse_button_listener(MouseButtonEventHandler handler, i32 priority = 0) = 0;
        virtual void remove_mouse_button_listener(InputListenerHandle handle) = 0;

        virtual InputListenerHandle add_mouse_move_listener(MouseMoveEventHandler handler) = 0;
        virtual void remove_mouse_move_listener(InputListenerHandle handle) = 0;

        virtual InputListenerHandle add_scroll_listener(MouseScrollEventHandler handler, i32 priority = 0) = 0;
        virtual void remove_scroll_listener(InputListenerHandle handle) = 0;

        virtual void update() = 0;

        static std::unique_ptr<Input> create();

      protected:
        Input() = default;
    };
} // namespace star::platform
