#pragma once

#include <unordered_set>

#include "editor_input_manager.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/platform/window.hpp"
#include "star/ecs/components/camera.hpp"

namespace star::editor {
    class ViewportCameraSystem {
      public:
        f32 move_speed = 5.0f;
        f32 look_sensitivity = 0.003f;
        f32 pan_sensitivity = 0.01f;
        f32 zoom_speed = 0.5f;
        f32 speed_boost = 3.0f;

        ~ViewportCameraSystem();

        void initialize(const components::Transform& transform);
        void attach(EditorInputManager& input, platform::Window* window = nullptr);
        void detach();
        void update(f32 dt, const flecs::world& world);

      private:
        enum class Mode {
            Idle,
            Fly,
            Orbit,
            Pan
        };

        static constexpr f32 MAX_PITCH = 1.553f;

        void on_key(const platform::KeyEvent& e);
        void on_mouse_button(const platform::MouseButtonEvent& e);
        void on_mouse_move(const platform::MouseMoveEvent& e);
        void on_scroll(const platform::MouseScrollEvent& e);
        void on_focus_lost();

        void re_sync_from(const components::Transform& transform);
        void apply_look(const components::Transform& transform);
        void apply_movement(f32 dt, const components::Transform& transform) const;
        void apply_orbit(const components::Transform& transform);
        void apply_pan(const components::Transform& transform) const;
        void rebuild_rotation(const components::Transform& transform) const;

        Mode m_mode = Mode::Idle;
        platform::Window* m_window = nullptr;
        EditorInputManager* m_input = nullptr;
        Vector2 m_frame_delta{};
        f32 m_scroll = 0.0f;
        f32 m_yaw = 0.0f;
        f32 m_pitch = 0.0f;
        bool m_alt_held = false;
        bool m_sync_on_entry = false;

        EventListenerHandle m_key_handle;
        EventListenerHandle m_mouse_button_handle;
        EventListenerHandle m_mouse_move_handle;
        EventListenerHandle m_scroll_handle;
        EventListenerHandle m_focus_lost_handle;

        std::unordered_set<platform::KeyCode> m_held_keys;
    };
} // namespace star::editor
