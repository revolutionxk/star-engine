#include "viewport_camera_system.hpp"

#include <cmath>

#include <flecs.h>

#include "star/math/math.hpp"

namespace star::editor {
    ViewportCameraSystem::~ViewportCameraSystem() {
        if (m_input)
            detach();
    }

    void ViewportCameraSystem::initialize(const components::Transform& transform) {
        re_sync_from(transform);
    }

    void ViewportCameraSystem::attach(EditorInputManager& input, platform::Window* window) {
        m_window = window;
        m_input = &input;

        m_key_handle = input.add_key_listener([this](const platform::KeyEvent& e) {
            on_key(e);
            return false;
        });
        m_mouse_button_handle = input.add_mouse_button_listener([this](const platform::MouseButtonEvent& e) {
            on_mouse_button(e);
            return false;
        });
        m_mouse_move_handle =
            input.add_mouse_move_listener([this](const platform::MouseMoveEvent& e) { on_mouse_move(e); });
        m_scroll_handle = input.add_scroll_listener([this](const platform::MouseScrollEvent& e) {
            on_scroll(e);
            return false;
        });
        m_focus_lost_handle = input.add_focus_lost_listener([this] { on_focus_lost(); });
    }

    void ViewportCameraSystem::detach() {
        if (!m_input)
            return;
        m_input->remove_key_listener(m_key_handle);
        m_input->remove_mouse_button_listener(m_mouse_button_handle);
        m_input->remove_mouse_move_listener(m_mouse_move_handle);
        m_input->remove_scroll_listener(m_scroll_handle);
        m_input->remove_focus_lost_listener(m_focus_lost_handle);
        m_input = nullptr;
        m_window = nullptr;
    }

    void ViewportCameraSystem::update(const f32 dt, components::Transform& transform) {
        if (m_sync_on_entry) {
            m_sync_on_entry = false;
            re_sync_from(transform);
        }

        switch (m_mode) {
            case Mode::Fly:
                apply_look(transform);
                apply_movement(dt, transform);
                break;
            case Mode::Orbit:
                apply_orbit(transform);
                break;
            case Mode::Pan:
                apply_pan(transform);
                break;
            case Mode::Idle:
                break;
        }
        if (m_scroll != 0.0f)
            transform.position = transform.position + transform.forward() * (m_scroll * zoom_speed);

        m_frame_delta = {};
        m_scroll = 0.0f;
    }

    void ViewportCameraSystem::on_key(const platform::KeyEvent& e) {
        using KC = platform::KeyCode;
        using IA = platform::InputAction;

        if (e.action == IA::Press || e.action == IA::Repeat)
            m_held_keys.insert(e.key);
        else
            m_held_keys.erase(e.key);

        m_alt_held = m_held_keys.contains(KC::LeftAlt) || m_held_keys.contains(KC::RightAlt);
    }

    void ViewportCameraSystem::on_mouse_button(const platform::MouseButtonEvent& e) {
        const bool pressed = (e.action == platform::InputAction::Press);

        if (e.button == platform::MouseButton::Right) {
            if (pressed && m_mode != Mode::Fly) {
                m_mode = Mode::Fly;
                m_sync_on_entry = true;
                m_frame_delta = {};
                if (m_window)
                    m_window->set_relative_mouse_mode(true);
                if (m_input)
                    m_input->set_mouse_captured(true);
            } else if (!pressed && m_mode == Mode::Fly) {
                m_mode = Mode::Idle;
                if (m_window)
                    m_window->set_relative_mouse_mode(false);
                if (m_input)
                    m_input->set_mouse_captured(false);
            }
        } else if (e.button == platform::MouseButton::Left && m_alt_held) {
            if (pressed && m_mode != Mode::Orbit) {
                m_mode = Mode::Orbit;
                m_sync_on_entry = true;
                m_frame_delta = {};
            } else if (!pressed && m_mode == Mode::Orbit) {
                m_mode = Mode::Idle;
            }
        } else if (e.button == platform::MouseButton::Middle) {
            m_mode = pressed ? Mode::Pan : (m_mode == Mode::Pan ? Mode::Idle : m_mode);
        }
    }

    void ViewportCameraSystem::on_mouse_move(const platform::MouseMoveEvent& e) {
        if (m_mode != Mode::Idle)
            m_frame_delta = {m_frame_delta.x + e.delta.x, m_frame_delta.y - e.delta.y};
    }

    void ViewportCameraSystem::on_scroll(const platform::MouseScrollEvent& e) {
        m_scroll -= e.offset.y;
    }

    void ViewportCameraSystem::on_focus_lost() {
        if (m_mode == Mode::Fly) {
            if (m_window)
                m_window->set_relative_mouse_mode(false);
            if (m_input)
                m_input->set_mouse_captured(false);
        }
        m_held_keys.clear();
        m_mode = Mode::Idle;
        m_alt_held = false;
        m_frame_delta = {};
    }

    void ViewportCameraSystem::re_sync_from(const components::Transform& transform) {
        const Vector3 fwd = transform.rotation * Vector3::forward();
        m_yaw = std::atan2(fwd.x, fwd.z);
        m_pitch = -std::asin(clamp(fwd.y, -1.0f, 1.0f));
    }

    void ViewportCameraSystem::apply_look(components::Transform& transform) {
        m_yaw -= m_frame_delta.x * look_sensitivity;
        m_pitch += m_frame_delta.y * look_sensitivity;
        m_pitch = clamp(m_pitch, -MAX_PITCH, MAX_PITCH);
        rebuild_rotation(transform);
    }

    void ViewportCameraSystem::apply_movement(const f32 dt, components::Transform& transform) const {
        const Vector3 fwd = -transform.forward();
        const Vector3 right = transform.rotation * Vector3::right();

        Vector3 move{};
        if (m_held_keys.contains(platform::KeyCode::W))
            move = move + fwd;
        if (m_held_keys.contains(platform::KeyCode::S))
            move = move - fwd;
        if (m_held_keys.contains(platform::KeyCode::D))
            move = move + right;
        if (m_held_keys.contains(platform::KeyCode::A))
            move = move - right;
        if (m_held_keys.contains(platform::KeyCode::E))
            move = move + Vector3::up();
        if (m_held_keys.contains(platform::KeyCode::Q))
            move = move - Vector3::up();

        if (move.length_squared() > 0.0f) {
            const f32 boost = m_held_keys.contains(platform::KeyCode::LeftShift) ||
                                      m_held_keys.contains(platform::KeyCode::RightShift)
                                  ? speed_boost
                                  : 1.0f;
            transform.position = transform.position + move.normalized() * (move_speed * boost * dt);
        }
    }

    void ViewportCameraSystem::apply_orbit(components::Transform& transform) {
        constexpr f32 orbit_dist = 5.0f;
        const Vector3 pivot = transform.position - transform.forward() * orbit_dist;

        m_yaw += m_frame_delta.x * look_sensitivity;
        m_pitch += m_frame_delta.y * look_sensitivity;
        m_pitch = clamp(m_pitch, -MAX_PITCH, MAX_PITCH);
        rebuild_rotation(transform);

        transform.position = pivot + transform.forward() * orbit_dist;
    }

    void ViewportCameraSystem::apply_pan(components::Transform& transform) const {
        const Vector3 right = transform.rotation * Vector3::right();
        const Vector3 up = transform.rotation * Vector3::up();
        transform.position =
            transform.position - right * (m_frame_delta.x * pan_sensitivity) + up * (m_frame_delta.y * pan_sensitivity);
    }

    void ViewportCameraSystem::rebuild_rotation(components::Transform& transform) const {
        const Quaternion q_yaw(Vector3{0.0f, 1.0f, 0.0f}, m_yaw);
        const Quaternion q_pitch(Vector3{1.0f, 0.0f, 0.0f}, m_pitch);
        transform.rotation = (q_yaw * q_pitch).normalized();
    }
} // namespace star::editor
