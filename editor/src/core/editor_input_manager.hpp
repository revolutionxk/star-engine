#pragma once

#include "star/core/event_dispatcher.hpp"
#include "star/platform/input/input.hpp"

namespace star::editor {
    class EditorInputManager {
      public:
        using InputListenerHandle = EventListenerHandle;
        using KeyEventHandler = platform::Input::KeyEventHandler;
        using MouseButtonEventHandler = platform::Input::MouseButtonEventHandler;
        using MouseMoveEventHandler = platform::Input::MouseMoveEventHandler;
        using ScrollEventHandler = platform::Input::MouseScrollEventHandler;

        EditorInputManager() = default;

        void initialize(platform::Input& input) {
            m_platform_input = &input;

            m_key_handle = input.add_key_listener([this](const platform::KeyEvent& e) {
                if (!m_viewport_focused)
                    return false;
                return m_key_dispatcher.dispatch(e);
            });
            m_mouse_button_handle = input.add_mouse_button_listener([this](const platform::MouseButtonEvent& e) {
                if (!m_viewport_hovered && !m_mouse_captured)
                    return false;
                return m_mouse_button_dispatcher.dispatch(e);
            });
            m_mouse_move_handle = input.add_mouse_move_listener([this](const platform::MouseMoveEvent& e) {
                if (!m_viewport_hovered && !m_mouse_captured)
                    return;
                m_mouse_move_dispatcher.dispatch(e);
            });
            m_scroll_handle = input.add_scroll_listener([this](const platform::MouseScrollEvent& e) {
                if (!m_viewport_hovered)
                    return false;
                return m_scroll_dispatcher.dispatch(e);
            });
        }

        void shutdown() {
            if (!m_platform_input)
                return;
            m_platform_input->remove_key_listener(m_key_handle);
            m_platform_input->remove_mouse_button_listener(m_mouse_button_handle);
            m_platform_input->remove_mouse_move_listener(m_mouse_move_handle);
            m_platform_input->remove_scroll_listener(m_scroll_handle);
            m_platform_input = nullptr;
        }

        InputListenerHandle add_key_listener(KeyEventHandler handler, const i32 priority = 0) {
            return m_key_dispatcher.add(std::move(handler), priority);
        }

        void remove_key_listener(const InputListenerHandle handle) {
            m_key_dispatcher.remove(handle);
        }

        InputListenerHandle add_mouse_button_listener(MouseButtonEventHandler handler, const i32 priority = 0) {
            return m_mouse_button_dispatcher.add(std::move(handler), priority);
        }

        void remove_mouse_button_listener(const InputListenerHandle handle) {
            m_mouse_button_dispatcher.remove(handle);
        }

        InputListenerHandle add_mouse_move_listener(MouseMoveEventHandler handler) {
            return m_mouse_move_dispatcher.add(std::move(handler));
        }

        void remove_mouse_move_listener(const InputListenerHandle handle) {
            m_mouse_move_dispatcher.remove(handle);
        }

        InputListenerHandle add_scroll_listener(ScrollEventHandler handler, const i32 priority = 0) {
            return m_scroll_dispatcher.add(std::move(handler), priority);
        }

        void remove_scroll_listener(const InputListenerHandle handle) {
            m_scroll_dispatcher.remove(handle);
        }

        InputListenerHandle add_focus_lost_listener(std::function<void()> handler) {
            const InputListenerHandle handle{++m_focus_lost_next_id};
            m_focus_lost_listeners.push_back({handle, std::move(handler)});
            return handle;
        }

        void remove_focus_lost_listener(InputListenerHandle handle) {
            const auto it = std::ranges::find_if(
                m_focus_lost_listeners, [handle](const FocusLostEntry& e) { return e.handle.id == handle.id; });
            if (it != m_focus_lost_listeners.end())
                m_focus_lost_listeners.erase(it);
        }

        void set_viewport_focused(const bool focused) noexcept {
            if (m_viewport_focused && !focused)
                fire_focus_lost();
            m_viewport_focused = focused;
        }

        void set_viewport_hovered(const bool hovered) noexcept {
            m_viewport_hovered = hovered;
        }

        void set_mouse_captured(const bool captured) noexcept {
            m_mouse_captured = captured;
        }

        [[nodiscard]] bool is_viewport_focused() const noexcept {
            return m_viewport_focused;
        }

        [[nodiscard]] bool is_viewport_hovered() const noexcept {
            return m_viewport_hovered;
        }

        [[nodiscard]] bool is_mouse_captured() const noexcept {
            return m_mouse_captured;
        }

      private:
        void fire_focus_lost() const {
            for (const auto& [handle, handler] : m_focus_lost_listeners)
                handler();
        }

        bool m_viewport_focused = false;
        bool m_viewport_hovered = false;
        bool m_mouse_captured = false;

        platform::Input* m_platform_input = nullptr;
        platform::Input::InputListenerHandle m_key_handle;
        platform::Input::InputListenerHandle m_mouse_button_handle;
        platform::Input::InputListenerHandle m_mouse_move_handle;
        platform::Input::InputListenerHandle m_scroll_handle;

        EventDispatcher<platform::KeyEvent> m_key_dispatcher;
        EventDispatcher<platform::MouseButtonEvent> m_mouse_button_dispatcher;
        VoidEventDispatcher<platform::MouseMoveEvent> m_mouse_move_dispatcher;
        EventDispatcher<platform::MouseScrollEvent> m_scroll_dispatcher;

        struct FocusLostEntry {
            InputListenerHandle handle;
            std::function<void()> handler;
        };

        std::vector<FocusLostEntry> m_focus_lost_listeners;
        u32 m_focus_lost_next_id = 0;
    };
} // namespace star::editor
