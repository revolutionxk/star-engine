#pragma once

#include <algorithm>
#include <functional>
#include <vector>

#include "star/core/types.hpp"

namespace star {
    struct EventListenerHandle {
        u32 id = 0;

        [[nodiscard]] bool is_valid() const noexcept {
            return id != 0;
        }
    };

    template<typename Event>
    class EventDispatcher {
      public:
        using Handler = std::function<bool(const Event&)>;

        EventListenerHandle add(Handler handler, i32 priority = 0) {
            const EventListenerHandle handle{++m_next_id};
            auto it = std::lower_bound(m_entries.begin(), m_entries.end(), priority,
                                       [](const Entry& e, i32 p) { return e.priority > p; });
            m_entries.insert(it, {handle, priority, std::move(handler)});
            return handle;
        }

        void remove(EventListenerHandle handle) {
            auto it = std::find_if(m_entries.begin(), m_entries.end(),
                                   [handle](const Entry& e) { return e.handle.id == handle.id; });
            if (it != m_entries.end())
                m_entries.erase(it);
        }

        bool dispatch(const Event& event) const {
            for (const auto& entry : m_entries) {
                if (entry.handler(event))
                    return true;
            }
            return false;
        }

        [[nodiscard]] bool empty() const noexcept {
            return m_entries.empty();
        }

      private:
        struct Entry {
            EventListenerHandle handle;
            i32 priority{};
            Handler handler;
        };

        std::vector<Entry> m_entries;
        u32 m_next_id = 0;
    };

    template<typename Event>
    class VoidEventDispatcher {
      public:
        using Handler = std::function<void(const Event&)>;

        EventListenerHandle add(Handler handler) {
            const EventListenerHandle handle{++m_next_id};
            m_entries.push_back({handle, std::move(handler)});
            return handle;
        }

        void remove(EventListenerHandle handle) {
            auto it = std::find_if(m_entries.begin(), m_entries.end(),
                                   [handle](const Entry& e) { return e.handle.id == handle.id; });
            if (it != m_entries.end())
                m_entries.erase(it);
        }

        void dispatch(const Event& event) const {
            for (const auto& entry : m_entries)
                entry.handler(event);
        }

        [[nodiscard]] bool empty() const noexcept {
            return m_entries.empty();
        }

      private:
        struct Entry {
            EventListenerHandle handle;
            Handler handler;
        };

        std::vector<Entry> m_entries;
        u32 m_next_id = 0;
    };

} // namespace star
