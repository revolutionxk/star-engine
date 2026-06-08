#pragma once

#include <functional>

#include <flecs.h>

namespace star::scene {
    class Entity {
      public:
        Entity() = default;

        explicit Entity(flecs::entity e) noexcept : m_entity(e) {}

        template<typename T>
        Entity& set(T value) {
            m_entity.set<T>(std::move(value));
            return *this;
        }

        template<typename T>
        Entity& add() {
            m_entity.add<T>();
            return *this;
        }

        template<typename T>
        Entity& remove() {
            m_entity.remove<T>();
            return *this;
        }

        template<typename T>
        [[nodiscard]] const T* get() const {
            return m_entity.try_get<T>();
        }

        template<typename T>
        [[nodiscard]] T* get_mut() {
            return m_entity.try_get_mut<T>();
        }

        template<typename T>
        [[nodiscard]] const T* try_get() const {
            return m_entity.try_get<T>();
        }

        template<typename T>
        [[nodiscard]] bool has() const {
            return m_entity.has<T>();
        }

        Entity& child_of(const Entity& parent) {
            m_entity.child_of(parent.m_entity);
            return *this;
        }

        void each_child(const std::function<void(Entity)>& callback) const {
            m_entity.children([&](flecs::entity child) { callback(Entity{child}); });
        }

        void destruct() const {
            m_entity.destruct();
        }

        [[nodiscard]] std::string_view name() const {
            return {m_entity.name().c_str()};
        }

        [[nodiscard]] u64 id() const {
            return m_entity.id();
        }

        [[nodiscard]] bool is_valid() const {
            return m_entity.is_valid();
        }

        explicit operator bool() const noexcept {
            return is_valid();
        }

        bool operator==(const Entity& other) const noexcept {
            return m_entity == other.m_entity;
        }

        bool operator!=(const Entity& other) const noexcept {
            return m_entity != other.m_entity;
        }

        [[nodiscard]] flecs::entity raw() const noexcept {
            return m_entity;
        }

      private:
        flecs::entity m_entity;
    };

} // namespace star::scene
