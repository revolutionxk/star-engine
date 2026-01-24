#include "star/platform/time/clock.hpp"

#include <algorithm>
#include <thread>

namespace star::platform {
    Clock::Clock() : m_start_time(std::chrono::steady_clock::now()) {}

    void Clock::reset() {
        m_start_time = std::chrono::steady_clock::now();
    }

    f32 Clock::elapsed() const {
        const auto current = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current - m_start_time);
        return static_cast<f32>(duration.count()) / 1'000'000.0f;
    }

    f64 Clock::elapsed_millis() const {
        const auto current = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current - m_start_time);
        return static_cast<f64>(duration.count()) / 1'000.0;
    }

    u64 Clock::elapsed_micros() const {
        const auto current = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current - m_start_time);
        return static_cast<u64>(duration.count());
    }

    FrameTimer::FrameTimer(const u32 target_fps)
        : m_last_frame_time(std::chrono::steady_clock::now()), m_target_fps(target_fps) {
        if (target_fps > 0) {
            m_target_frame_time = 1.0f / static_cast<f32>(target_fps);
        }
    }

    f32 FrameTimer::tick() {
        const auto current_time = std::chrono::steady_clock::now();

        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - m_last_frame_time);
        m_delta_time = static_cast<f32>(duration.count()) / 1'000'000.0f;

        constexpr f32 MAX_DELTA_TIME = 0.1f;
        m_delta_time = std::min(m_delta_time, MAX_DELTA_TIME);
        m_smoothed_delta_time =
            m_smoothed_delta_time * (1.0f - DELTA_TIME_SMOOTHING) + m_delta_time * DELTA_TIME_SMOOTHING;

        m_fps_accumulator += m_delta_time;
        m_fps_frame_count++;

        if (m_fps_accumulator >= FPS_UPDATE_INTERVAL) {
            m_current_fps = static_cast<f32>(m_fps_frame_count) / m_fps_accumulator;
            m_fps_accumulator = 0.0f;
            m_fps_frame_count = 0;
        }

        if (m_target_fps > 0) {
            if (const auto actual_frame_time = m_delta_time; actual_frame_time < m_target_frame_time) {
                const auto sleep_duration = std::chrono::microseconds(
                    static_cast<u64>((m_target_frame_time - actual_frame_time) * 1'000'000.0f));
                std::this_thread::sleep_for(sleep_duration);

                const auto new_time = std::chrono::steady_clock::now();
                const auto new_duration =
                    std::chrono::duration_cast<std::chrono::microseconds>(new_time - m_last_frame_time);
                m_delta_time = static_cast<f32>(new_duration.count()) / 1'000'000.0f;
            }
        }

        m_total_time += m_delta_time;
        m_frame_count++;
        m_last_frame_time = current_time;

        return m_delta_time;
    }

    f32 FrameTimer::fps() const {
        return m_current_fps;
    }

    void FrameTimer::set_target_fps(const u32 target_fps) {
        m_target_fps = target_fps;
        if (target_fps > 0) {
            m_target_frame_time = 1.0f / static_cast<f32>(target_fps);
        } else {
            m_target_frame_time = 0.0f;
        }
    }

    void FrameTimer::reset() {
        m_clock.reset();
        m_last_frame_time = std::chrono::steady_clock::now();
        m_delta_time = 0.0f;
        m_smoothed_delta_time = 0.0f;
        m_total_time = 0.0f;
        m_frame_count = 0;
        m_fps_accumulator = 0.0f;
        m_fps_frame_count = 0;
        m_current_fps = 0.0f;
    }

} // namespace star::platform
