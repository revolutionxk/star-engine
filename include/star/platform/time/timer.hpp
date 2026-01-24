#pragma once

#include <vector>

#include "clock.hpp"
#include "star/core/types.hpp"

namespace star::platform {
    class Timer {
      public:
        Timer() = default;

        void start();
        void stop();
        void pause();
        void resume();
        void reset();

        [[nodiscard]] bool is_running() const {
            return m_running && !m_paused;
        }

        [[nodiscard]] bool is_paused() const {
            return m_paused;
        }

        [[nodiscard]] f32 elapsed() const;

        [[nodiscard]] f64 elapsed_millis() const;

        [[nodiscard]] bool has_elapsed(const f32 seconds) const {
            return elapsed() >= seconds;
        }

        [[nodiscard]] f32 remaining(const f32 target) const {
            const f32 elapsed_time = elapsed();
            return elapsed_time >= target ? 0.0f : target - elapsed_time;
        }

        [[nodiscard]] f32 progress(f32 target) const;

      private:
        Clock m_clock;
        Clock::TimePoint m_pause_time{};

        f32 m_accumulated_time{0.0f};
        bool m_running{false};
        bool m_paused{false};
    };

    class Countdown {
      public:
        explicit Countdown(f32 duration, bool auto_start = false);

        void start(f32 duration = -1.0f);
        void stop();

        bool update(f32 dt);

        [[nodiscard]] bool is_finished() const;

        [[nodiscard]] bool is_running() const {
            return m_running;
        }

        [[nodiscard]] f32 remaining() const;
        [[nodiscard]] f32 elapsed() const;
        [[nodiscard]] f32 progress() const;

        [[nodiscard]] f32 duration() const {
            return m_duration;
        }

        void set_duration(f32 duration);
        void reset();
        void add_time(f32 seconds);

      private:
        Timer m_timer;
        f32 m_duration;
        bool m_running{false};
    };

    class Stopwatch {
      public:
        Stopwatch() = default;

        void start();
        void stop();
        f32 lap();

        void reset();

        [[nodiscard]] f32 elapsed() const {
            return m_timer.elapsed();
        }

        [[nodiscard]] bool is_running() const {
            return m_timer.is_running();
        }

        [[nodiscard]] const std::vector<f32>& laps() const {
            return m_laps;
        }

        [[nodiscard]] f32 last_lap() const;
        [[nodiscard]] f32 average_lap() const;
        [[nodiscard]] f32 fastest_lap() const;
        [[nodiscard]] f32 slowest_lap() const;

      private:
        Timer m_timer;
        std::vector<f32> m_laps;
        f32 m_last_lap_time{0.0f};
    };

} // namespace star::platform
