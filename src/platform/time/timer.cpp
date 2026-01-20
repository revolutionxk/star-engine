#include "star/platform/time/timer.hpp"

#include <algorithm>
#include <numeric>

namespace star::platform {
    void Timer::start() {
        m_running = true;
        m_paused = false;
        m_accumulated_time = 0.0f;
        m_clock.reset();
    }

    void Timer::stop() {
        m_running = false;
        m_paused = false;
        m_accumulated_time = 0.0f;
    }

    void Timer::pause() {
        if (!m_running || m_paused) {
            return;
        }

        m_paused = true;
        m_pause_time = Clock::now();
        m_accumulated_time += m_clock.elapsed();
    }

    void Timer::resume() {
        if (!m_running || !m_paused) {
            return;
        }

        m_paused = false;
        m_clock.reset();
    }

    void Timer::reset() {
        m_accumulated_time = 0.0f;
        m_clock.reset();
        if (m_paused) {
            m_pause_time = Clock::now();
        }
    }

    f32 Timer::elapsed() const {
        if (!m_running) {
            return 0.0f;
        }

        if (m_paused) {
            return m_accumulated_time;
        }

        return m_accumulated_time + m_clock.elapsed();
    }

    f64 Timer::elapsed_millis() const {
        return static_cast<f64>(elapsed()) * 1000.0;
    }

    f32 Timer::progress(const f32 target) const {
        if (target <= 0.0f) {
            return 1.0f;
        }

        const f32 elapsed_time = elapsed();
        return std::clamp(elapsed_time / target, 0.0f, 1.0f);
    }

    Countdown::Countdown(const f32 duration, const bool auto_start) : m_duration(duration) {
        if (auto_start) {
            start();
        }
    }

    void Countdown::start(const f32 duration) {
        if (duration >= 0.0f) {
            m_duration = duration;
        }

        m_running = true;
        m_timer.start();
    }

    void Countdown::stop() {
        m_running = false;
        m_timer.stop();
    }

    bool Countdown::update(const f32 dt) {
        if (!m_running) {
            return false;
        }

        if (const bool was_finished = is_finished(); !was_finished && is_finished()) {
            m_running = false;
            return true;
        }

        return false;
    }

    bool Countdown::is_finished() const {
        if (!m_running) {
            return true;
        }

        return m_timer.elapsed() >= m_duration;
    }

    f32 Countdown::remaining() const {
        if (!m_running) {
            return 0.0f;
        }

        const f32 elapsed_time = m_timer.elapsed();
        return elapsed_time >= m_duration ? 0.0f : m_duration - elapsed_time;
    }

    f32 Countdown::elapsed() const {
        return m_timer.elapsed();
    }

    f32 Countdown::progress() const {
        if (m_duration <= 0.0f) {
            return 1.0f;
        }

        return std::clamp(m_timer.elapsed() / m_duration, 0.0f, 1.0f);
    }

    void Countdown::set_duration(const f32 duration) {
        m_duration = duration;
    }

    void Countdown::reset() {
        m_timer.reset();
    }

    void Countdown::add_time(const f32 seconds) {
        m_duration += seconds;
        if (m_duration < 0.0f) {
            m_duration = 0.0f;
        }
    }

    void Stopwatch::start() {
        m_timer.start();
        m_last_lap_time = 0.0f;
    }

    void Stopwatch::stop() {
        m_timer.stop();
    }

    f32 Stopwatch::lap() {
        const f32 current_time = m_timer.elapsed();
        const f32 lap_time = current_time - m_last_lap_time;
        m_laps.push_back(lap_time);
        m_last_lap_time = current_time;
        return lap_time;
    }

    void Stopwatch::reset() {
        m_timer.reset();
        m_laps.clear();
        m_last_lap_time = 0.0f;
    }

    f32 Stopwatch::last_lap() const {
        return m_laps.empty() ? 0.0f : m_laps.back();
    }

    f32 Stopwatch::average_lap() const {
        if (m_laps.empty()) {
            return 0.0f;
        }

        const f32 sum = std::accumulate(m_laps.begin(), m_laps.end(), 0.0f);
        return sum / static_cast<f32>(m_laps.size());
    }

    f32 Stopwatch::fastest_lap() const {
        if (m_laps.empty()) {
            return 0.0f;
        }

        return *std::ranges::min_element(m_laps);
    }

    f32 Stopwatch::slowest_lap() const {
        if (m_laps.empty()) {
            return 0.0f;
        }

        return *std::ranges::max_element(m_laps);
    }

} // namespace star::platform
