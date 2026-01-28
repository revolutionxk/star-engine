#pragma once

#include <chrono>

#include "star/core/types.hpp"

namespace star::platform {
    class Clock {
      public:
        using TimePoint = std::chrono::steady_clock::time_point;
        using Duration = std::chrono::steady_clock::duration;

        Clock();

        void reset();

        [[nodiscard]] f32 elapsed() const;
        [[nodiscard]] f64 elapsed_millis() const;
        [[nodiscard]] u64 elapsed_micros() const;

        [[nodiscard]] TimePoint start_time() const {
            return m_start_time;
        }

        [[nodiscard]] static TimePoint now() {
            return std::chrono::steady_clock::now();
        }

      private:
        TimePoint m_start_time;
    };

    class FrameTimer {
      public:
        using TimePoint = std::chrono::steady_clock::time_point;

        explicit FrameTimer(u32 target_fps = 0);

        f32 tick();

        [[nodiscard]] f32 delta_time() const {
            return m_delta_time;
        }

        [[nodiscard]] f32 smoothed_delta_time() const {
            return m_smoothed_delta_time;
        }

        [[nodiscard]] f32 fps() const;

        void set_target_fps(u32 target_fps);

        [[nodiscard]] u32 target_fps() const {
            return m_target_fps;
        }

        [[nodiscard]] f32 total_time() const {
            return m_total_time;
        }

        [[nodiscard]] u64 frame_count() const {
            return m_frame_count;
        }

        void reset();

    private:
        Clock m_clock;
        TimePoint m_last_frame_time;

        f32 m_delta_time{0.0f};
        f32 m_smoothed_delta_time{0.0f};
        f32 m_total_time{0.0f};

        u32 m_target_fps{0};
        f32 m_target_frame_time{0.0f};

        u64 m_frame_count{0};

        f32 m_fps_accumulator{0.0f};
        u32 m_fps_frame_count{0};
        f32 m_current_fps{0.0f};

        static constexpr f32 FPS_UPDATE_INTERVAL = 0.5f;
        static constexpr f32 DELTA_TIME_SMOOTHING = 0.1f;
    };

} // namespace star::platform
