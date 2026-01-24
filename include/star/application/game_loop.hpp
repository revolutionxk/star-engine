#pragma once

#include <functional>

#include "star/core/types.hpp"
#include "star/platform/time/clock.hpp"

namespace star::application {
    struct GameLoopConfig {
        f32 fixed_timestep{1.0f / 60.0f};
        f32 max_frame_time{0.1f};
        bool use_fixed_timestep{false};
        u32 max_updates_per_frame{5};
        u32 target_fps{0};
    };

    class STAR_EXPORT GameLoop {
      public:
        using UpdateCallback = std::function<void(f32 delta_time)>;
        using RenderCallback = std::function<void()>;

        explicit GameLoop(const GameLoopConfig& config = {});
        ~GameLoop() = default;

        GameLoop(const GameLoop&) = delete;
        GameLoop& operator=(const GameLoop&) = delete;

        void run(const UpdateCallback& update_fn, const RenderCallback& render_fn,
                 const std::function<bool()>& should_continue);

        f32 tick();

        [[nodiscard]] f32 delta_time() const {
            return m_delta_time;
        }

        [[nodiscard]] f32 fps() const;
        void set_target_fps(u32 fps);

        [[nodiscard]] const GameLoopConfig& config() const {
            return m_config;
        }

        void set_config(const GameLoopConfig& config);

      private:
        void execute_fixed_updates(const UpdateCallback& update_fn);

        GameLoopConfig m_config;
        platform::FrameTimer m_frame_timer;
        f32 m_delta_time{0.0f};
        f32 m_fixed_time_accumulator{0.0f};
    };
} // namespace star::application
