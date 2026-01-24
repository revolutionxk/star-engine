#pragma once

namespace star::resources {
    enum class ResourceState {
        Unloaded,
        Loading,
        Loaded,
        Failed
    };

    class Resource {
        friend class ResourceManager;

      public:
        virtual ~Resource() = default;

        const std::string& path() const {
            return m_path;
        }

        ResourceState state() const {
            return m_state;
        }

        u32 generation() const {
            return m_generation;
        }

        bool is_loaded() const {
            return m_state == ResourceState::Loaded;
        }

      protected:
        std::string m_path{};
        ResourceState m_state{ResourceState::Unloaded};
        u32 m_generation{0};
    };
} // namespace star::resources
