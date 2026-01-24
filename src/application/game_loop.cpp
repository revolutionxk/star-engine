#include "star/application/game_loop.hpp"

namespace star::application {

    GameLoop::GameLoop(const GameLoopConfig& config) : m_config(config) {
        if (m_config.target_fps > 0) {
            m_frame_timer.set_target_fps(m_config.target_fps);
        }
    }

    void GameLoop::run(const UpdateCallback& update_fn, const RenderCallback& render_fn,
                       const std::function<bool()>& should_continue) {
        while (should_continue()) {
            const f32 dt = tick();

            if (m_config.use_fixed_timestep) {
                execute_fixed_updates(update_fn);
            } else {
                update_fn(dt);
            }

            if (render_fn) {
                render_fn();
            }
        }
    }

    f32 GameLoop::tick() {
        m_delta_time = m_frame_timer.tick();

        if (m_delta_time > m_config.max_frame_time) {
            m_delta_time = m_config.max_frame_time;
        }

        return m_delta_time;
    }

    void GameLoop::execute_fixed_updates(const UpdateCallback& update_fn) {
        m_fixed_time_accumulator += m_delta_time;

        u32 update_count = 0;
        while (m_fixed_time_accumulator >= m_config.fixed_timestep && update_count < m_config.max_updates_per_frame) {
            update_fn(m_config.fixed_timestep);
            m_fixed_time_accumulator -= m_config.fixed_timestep;
            ++update_count;
        }

        if (update_count >= m_config.max_updates_per_frame) {
            m_fixed_time_accumulator = 0.0f;
        }
    }

    f32 GameLoop::fps() const {
        return m_frame_timer.fps();
    }

    void GameLoop::set_target_fps(const u32 fps) {
        m_config.target_fps = fps;
        m_frame_timer.set_target_fps(fps);
    }

    void GameLoop::set_config(const GameLoopConfig& config) {
        m_config = config;
        if (m_config.target_fps > 0) {
            m_frame_timer.set_target_fps(m_config.target_fps);
        }
    }

} // namespace star::application
