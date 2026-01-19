#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "star/platform/window.hpp"

namespace star::application {
    using WindowId = u32;
    constexpr WindowId INVALID_WINDOW_ID = 0;

    class STAR_EXPORT WindowManager {
      public:
        WindowManager() = default;
        ~WindowManager() = default;

        WindowManager(const WindowManager&) = delete;
        WindowManager& operator=(const WindowManager&) = delete;

        WindowId create_window(const platform::VideoMode& video_mode, bool is_main = false);

        bool destroy_window(WindowId window_id);

        platform::Window* get_window(WindowId window_id) const;

        platform::Window* get_main_window() const;

        WindowId get_main_window_id() const {
            return m_main_window_id;
        }

        bool set_main_window(WindowId window_id);

        std::vector<WindowId> get_all_window_ids() const;

        size_t get_window_count() const {
            return m_windows.size();
        }

        bool has_open_windows() const;
        void poll_events() const;
        void destroy_all();

      private:
        struct WindowData {
            std::unique_ptr<platform::Window> window;
            bool is_main = false;
        };

        std::unordered_map<WindowId, WindowData> m_windows;
        WindowId m_main_window_id = INVALID_WINDOW_ID;
        WindowId m_next_window_id = 1;
    };
} // namespace star::application
