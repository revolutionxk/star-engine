#include "star/application/game_loop.hpp"

namespace star::application {

    GameLoop::GameLoop(const GameLoopConfig& config) : m_config(config) {
        if (m_config.target_fps > 0) {
            m_frame_timer.set_target_fps(m_config.target_fps);
        }
    }

    void GameLoop::run(const FixedUpdateCallback& fixed_update, const UpdateCallback& update,
                       const RenderCallback& render, const std::function<bool()>& should_continue) {
        while (should_continue()) {
            const f32 dt = tick();

            if (update) {
                update(dt);
            }

            if (m_config.fixed_timestep > 0.0f) {
                m_accumulator += dt;

                u32 steps = 0;
                while (m_accumulator >= m_config.fixed_timestep && steps < m_config.max_fixed_steps) {
                    if (fixed_update) {
                        fixed_update(m_config.fixed_timestep);
                    }
                    m_accumulator -= m_config.fixed_timestep;
                    ++steps;
                }
                
                if (steps >= m_config.max_fixed_steps) {
                    m_accumulator = 0.0f;
                }

                m_alpha = m_accumulator / m_config.fixed_timestep;
            }

            if (render) {
                render(m_alpha);
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
