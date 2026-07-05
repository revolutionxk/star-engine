#pragma once

#include <string>
#include <string_view>

#include <flecs.h>

#include "star/core/types.hpp"

namespace star::ecs {
    enum class Phase {
        OnLoad,
        PreUpdate,
        OnUpdate,
        PostUpdate,
        PreRender,
    };

    class World {
      public:
        World() = default;

        template<typename... Components, typename Fn>
        void add_system(const std::string_view name, const Phase phase, Fn&& fn) {
            m_world.system<Components...>(std::string(name).c_str())
                .kind(phase_entity(phase))
                .each(std::forward<Fn>(fn));
        }

        void progress(const f32 dt) const {
            m_world.progress(dt);
        }

        [[nodiscard]] flecs::world& native() noexcept {
            return m_world;
        }

        [[nodiscard]] const flecs::world& native() const noexcept {
            return m_world;
        }

      private:
        static flecs::entity_t phase_entity(const Phase phase) {
            switch (phase) {
                case Phase::OnLoad:
                    return flecs::OnLoad;
                case Phase::PreUpdate:
                    return flecs::PreUpdate;
                case Phase::PostUpdate:
                    return flecs::PostUpdate;
                case Phase::PreRender:
                    return flecs::PreStore;
                case Phase::OnUpdate:
                default:
                    return flecs::OnUpdate;
            }
        }

        flecs::world m_world;
    };
} // namespace star::ecs
