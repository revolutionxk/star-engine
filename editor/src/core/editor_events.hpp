#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "flecs.h"

namespace star::editor {
    enum class EditorEventType {
        EntitySelected,
        EntityDeselected,
        SceneLoaded,
        SceneSaved,
        ViewportResized
    };

    struct EditorEvent {
        EditorEventType type;
        void* data = nullptr;

        template<typename T>
        [[nodiscard]] T* get_data() const {
            return static_cast<T*>(data);
        }
    };

    struct NodeSelectedEvent {
        flecs::entity& node;
    };

    class EditorEventBus {
      public:
        using EventCallback = std::function<void(const EditorEvent&)>;

        static EditorEventBus& instance() {
            static EditorEventBus s_instance;
            return s_instance;
        }

        void subscribe(const EditorEventType type, EventCallback callback) {
            m_subscribers[type].push_back(std::move(callback));
        }

        void publish(const EditorEvent& event) {
            if (const auto it = m_subscribers.find(event.type); it != m_subscribers.end()) {
                for (const auto& callback : it->second) {
                    callback(event);
                }
            }
        }

        void clear() {
            m_subscribers.clear();
        }

      private:
        EditorEventBus() = default;
        std::unordered_map<EditorEventType, std::vector<EventCallback>> m_subscribers;
    };
} // namespace star::editor
