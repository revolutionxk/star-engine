#include "panel_manager.hpp"

namespace star::editor {
    Panel* PanelManager::get_panel_by_name(const std::string& name) const {
        if (const auto it = m_panels.find(name); it != m_panels.end())
            return it->second.get();
        return nullptr;
    }

    void PanelManager::update_all(const f32 dt) const {
        for (auto* panel : m_panel_order) {
            if (panel && panel->is_open())
                panel->on_update(dt);
        }
    }

    void PanelManager::render_all() const {
        for (auto* panel : m_panel_order) {
            if (panel)
                panel->on_imgui_render();
        }
    }

    void PanelManager::shutdown() {
        for (const auto& panel : m_panels | std::views::values)
            panel->on_detach();
        m_panel_order.clear();
        m_type_index.clear();
        m_panels.clear();
    }
} // namespace star::editor
