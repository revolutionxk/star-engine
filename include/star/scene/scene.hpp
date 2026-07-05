#pragma once
#include <memory>
#include <string>
#include <vector>

#include <flecs.h>

#include "star/ecs/world.hpp"
#include "star/scene/entity.hpp"

namespace star::scene {
    class STAR_EXPORT Scene {
      public:
        explicit Scene(const std::string& name);
        ~Scene();

        [[nodiscard]] Entity create_entity(const std::string& name = "") const;
        [[nodiscard]] Entity find_entity(const std::string& name) const;
        [[nodiscard]] Entity find_by_path(const std::string& path) const;

        void ready();
        void update(float dt);
        void shutdown();

        [[nodiscard]] const std::string& name() const {
            return m_name;
        }

        [[nodiscard]] bool is_active() const {
            return m_active;
        }

        void set_active(const bool active) {
            m_active = active;
        }

        [[nodiscard]] bool is_ready() const {
            return m_is_ready;
        }

        ecs::World& world() {
            return m_world;
        }

        [[nodiscard]] flecs::entity root() const {
            return m_root;
        }

      private:
        std::string m_name;
        ecs::World m_world;
        flecs::entity m_root;

        bool m_active{true};
        bool m_is_ready{false};
    };
} // namespace star::scene
