#pragma once
#include <string>
#include <vector>

#include <flecs.h>

namespace star::scene {
    class Node {
      public:
        Node(flecs::world& world, const std::string& name);
        virtual ~Node() = default;

        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;
        Node(Node&&) noexcept = default;
        Node& operator=(Node&&) noexcept = delete;

        virtual void on_ready() {}

        virtual void on_update(float dt) {}

        virtual void on_exit() {}

        void add_child(Node* child);
        void remove_child(Node* child);

        Node* get_parent() const {
            return m_parent;
        }

        const std::vector<Node*>& get_children() const {
            return m_children;
        }

        Node* find_child(const std::string& name) const;

        template<typename T>
        T* find_child_by_type();

        std::optional<const flecs::entity*> entity() const {
            return &m_entity;
        }

        const std::string& name() const {
            return m_name;
        }

        void set_name(const std::string& name) {
            m_name = name;
        }

        template<typename T>
        T& get_component() {
            return m_entity.get_mut<T>();
        }

        template<typename T>
        void add_component(const T& component) {
            m_entity.set<T>(component);
        }

      protected:
        flecs::world& m_world;
        flecs::entity m_entity{};
        std::string m_name;

        Node* m_parent{nullptr};
        std::vector<Node*> m_children;
    };
} // namespace star::scene
