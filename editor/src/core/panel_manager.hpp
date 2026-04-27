#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "../panels/panel.hpp"
#include "star/core/common.hpp"

namespace star::editor {
    class PanelManager {
      public:
        PanelManager() = default;
        ~PanelManager() = default;

        PanelManager(const PanelManager&) = delete;
        PanelManager& operator=(const PanelManager&) = delete;
        PanelManager(PanelManager&&) = default;
        PanelManager& operator=(PanelManager&&) = default;

        template<typename T, typename... Args>
        T* register_panel(Args&&... args) {
            static_assert(std::is_base_of_v<Panel, T>, "T must inherit from Panel");

            auto panel = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = panel.get();

            const std::string& name = panel->name();
            m_panels[name] = std::move(panel);
            m_panel_order.push_back(ptr);
            m_type_index[std::type_index(typeid(T))] = ptr;

            ptr->on_attach();
            return ptr;
        }

        template<typename T>
        [[nodiscard]] T* get_panel() {
            static_assert(std::is_base_of_v<Panel, T>, "T must inherit from Panel");

            auto it = m_type_index.find(std::type_index(typeid(T)));
            if (it != m_type_index.end())
                return static_cast<T*>(it->second);
            return nullptr;
        }

        [[nodiscard]] Panel* get_panel_by_name(const std::string& name) const;

        void update_all(f32 dt) const;
        void render_all() const;
        void shutdown();

        [[nodiscard]] const std::vector<Panel*>& get_all_panels() const {
            return m_panel_order;
        }

      private:
        std::unordered_map<std::string, std::unique_ptr<Panel>> m_panels;
        std::unordered_map<std::type_index, Panel*> m_type_index;
        std::vector<Panel*> m_panel_order;
    };
} // namespace star::editor
