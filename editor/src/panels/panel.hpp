#pragma once

#include <string>
#include <string_view>

#include "editor_window.hpp"
#include "star/core/common.hpp"

namespace star::editor {

    class Panel {
      public:
        explicit Panel(const std::string_view name) : m_name(name) {}

        virtual ~Panel() = default;

        Panel(const Panel&) = delete;
        Panel& operator=(const Panel&) = delete;
        Panel(Panel&&) = delete;
        Panel& operator=(Panel&&) = delete;

        virtual void on_attach() {}

        virtual void on_detach() {}

        virtual void on_update([[maybe_unused]] f32 dt) {}

        virtual void on_imgui_render() = 0;

        [[nodiscard]] const std::string& name() const noexcept {
            return m_name;
        }

        [[nodiscard]] bool is_open() const noexcept {
            return m_is_open;
        }

        void set_open(const bool open) noexcept {
            m_is_open = open;
        }

        void toggle() noexcept {
            m_is_open = !m_is_open;
        }

      protected:
        std::string m_name;
        bool m_is_open = true;
    };

} // namespace star::editor
