#pragma once
#include <memory>
#include <string>
#include <vector>

#include <flecs.h>

#include "node.hpp"

namespace star::scene {
    class Scene {
      public:
        explicit Scene(const std::string& name);
        ~Scene();

        template<typename T = Node>
        T* create_node(const std::string& name) {
            auto node = std::make_unique<T>(m_world, name);
            T* ptr = node.get();
            m_nodes.push_back(std::move(node));

            if (!ptr->get_parent()) {
                m_root_nodes.push_back(ptr);
            }

            if (m_is_ready) {
                ptr->on_ready();
            }

            return ptr;
        }

        Node* find_node(const std::string& path) const;
        Node* find_node_by_name(const std::string& name) const;

        template<typename T>
        std::vector<T*> find_nodes_by_type() {
            std::vector<T*> result;
            for (auto& node : m_nodes) {
                if (auto* typed = dynamic_cast<T*>(node.get())) {
                    result.push_back(typed);
                }
            }
            return result;
        }

        const std::vector<Node*>& get_root_nodes() const {
            return m_root_nodes;
        }

        void ready();
        void update(float dt) const;
        void shutdown();

        const std::string& name() const {
            return m_name;
        }

        bool is_active() const {
            return m_active;
        }

        void set_active(bool active) {
            m_active = active;
        }

        bool is_ready() const {
            return m_is_ready;
        }

        flecs::world& world() {
            return m_world;
        }

      private:
        void register_components();
        void register_systems();

        std::string m_name;
        flecs::world m_world;

        std::vector<std::unique_ptr<Node>> m_nodes;
        std::vector<Node*> m_root_nodes;

        bool m_active{true};
        bool m_is_ready{false};
    };
} // namespace star::scene
