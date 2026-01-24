#pragma once

#include "star/core/common.hpp"
#include "../panels/panel.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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
            T* panel_ptr = panel.get();
            
            const std::string& name = panel->name();
            m_panels[name] = std::move(panel);
            m_panel_order.push_back(panel_ptr);
            
            panel_ptr->on_attach();
            
            return panel_ptr;
        }

        template<typename T>
        [[nodiscard]] T* get_panel() {
            static_assert(std::is_base_of_v<Panel, T>, "T must inherit from Panel");
            
            for (auto& [name, panel] : m_panels) {
                if (auto* typed_panel = dynamic_cast<T*>(panel.get())) {
                    return typed_panel;
                }
            }
            return nullptr;
        }

        [[nodiscard]] Panel* get_panel_by_name(const std::string& name) {
            if (auto it = m_panels.find(name); it != m_panels.end()) {
                return it->second.get();
            }
            return nullptr;
        }

        void update_all(f32 dt) {
            for (auto* panel : m_panel_order) {
                if (panel && panel->is_open()) {
                    panel->on_update(dt);
                }
            }
        }

        void render_all() const {
            for (auto* panel : m_panel_order) {
                if (panel) {
                    panel->on_imgui_render();
                }
            }
        }

        void shutdown() {
            for (auto& [name, panel] : m_panels) {
                panel->on_detach();
            }
            m_panel_order.clear();
            m_panels.clear();
        }

        [[nodiscard]] const std::vector<Panel*>& get_all_panels() const {
            return m_panel_order;
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<Panel>> m_panels;
        std::vector<Panel*> m_panel_order;
    };

} // namespace star::editor
