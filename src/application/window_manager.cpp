#include "star/application/window_manager.hpp"

namespace star::application {
    WindowId WindowManager::create_window(const platform::VideoMode& video_mode, bool is_main) {
        auto window = platform::Window::create();
        if (!window) {
            STAR_LOG_ERROR(LogCategory::Application, "Failed to create platform window");
            return INVALID_WINDOW_ID;
        }

        if (!window->create(video_mode)) {
            STAR_LOG_ERROR(LogCategory::Application, "Failed to initialize window");
            return INVALID_WINDOW_ID;
        }

        WindowId window_id = m_next_window_id++;

        WindowData data;
        data.window = std::move(window);
        data.is_main = is_main;

        m_windows[window_id] = std::move(data);

        if (is_main || m_main_window_id == INVALID_WINDOW_ID) {
            m_main_window_id = window_id;
            m_windows[window_id].is_main = true;
        }

        STAR_LOG_INFO(LogCategory::Application, "Window created with ID: {} (Main: {})", window_id, is_main);

        return window_id;
    }

    bool WindowManager::destroy_window(WindowId window_id) {
        auto it = m_windows.find(window_id);
        if (it == m_windows.end()) {
            STAR_LOG_WARN(LogCategory::Application, "Attempted to destroy non-existent window: {}", window_id);
            return false;
        }

        if (it->second.window) {
            it->second.window->destroy();
        }

        const bool was_main = it->second.is_main;
        m_windows.erase(it);

        // If we destroyed the main window, set another window as main if available
        if (was_main && !m_windows.empty()) {
            m_main_window_id = m_windows.begin()->first;
            m_windows.begin()->second.is_main = true;
            STAR_LOG_INFO(LogCategory::Application, "Main window changed to ID: {}", m_main_window_id);
        } else if (m_windows.empty()) {
            m_main_window_id = INVALID_WINDOW_ID;
        }

        STAR_LOG_INFO(LogCategory::Application, "Window destroyed: {}", window_id);
        return true;
    }

    platform::Window* WindowManager::get_window(const WindowId window_id) const {
        const auto it = m_windows.find(window_id);
        return it != m_windows.end() ? it->second.window.get() : nullptr;
    }

    platform::Window* WindowManager::get_main_window() const {
        if (m_main_window_id == INVALID_WINDOW_ID) {
            return nullptr;
        }
        return get_window(m_main_window_id);
    }

    bool WindowManager::set_main_window(WindowId window_id) {
        const auto it = m_windows.find(window_id);
        if (it == m_windows.end()) {
            STAR_LOG_WARN(LogCategory::Application, "Attempted to set non-existent window as main: {}", window_id);
            return false;
        }

        if (m_main_window_id != INVALID_WINDOW_ID) {
            if (const auto old_main = m_windows.find(m_main_window_id); old_main != m_windows.end()) {
                old_main->second.is_main = false;
            }
        }
        m_main_window_id = window_id;
        it->second.is_main = true;

        STAR_LOG_INFO(LogCategory::Application, "Main window set to ID: {}", window_id);
        return true;
    }

    std::vector<WindowId> WindowManager::get_all_window_ids() const {
        std::vector<WindowId> ids;
        ids.reserve(m_windows.size());

        for (const auto& id : m_windows | std::views::keys) {
            ids.push_back(id);
        }

        return ids;
    }

    bool WindowManager::has_open_windows() const {
        for (const auto& [window, is_main] : m_windows | std::views::values) {
            if (window && window->is_opened()) {
                return true;
            }
        }
        return false;
    }

    void WindowManager::poll_events() const {
        for (const auto window_ids = get_all_window_ids(); const WindowId id : window_ids) {
            if (auto* window = get_window(id); window && window->is_opened()) {
                window->pool_events();
            }
        }
    }

    void WindowManager::destroy_all() {
        STAR_LOG_INFO(LogCategory::Application, "Destroying all windows ({} total)", m_windows.size());

        for (auto& [window, is_main] : m_windows | std::views::values) {
            if (window) {
                window->destroy();
            }
        }

        m_windows.clear();
        m_main_window_id = INVALID_WINDOW_ID;
    }
} // namespace star::application
