#pragma once

#include <cstddef>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "command.hpp"

namespace star::editor {
    class CommandStack {
      public:
        explicit CommandStack(const std::size_t max_depth = 100) : m_max_depth(max_depth) {}

        void push(std::unique_ptr<ICommand> command) {
            if (!command)
                return;

            command->execute();
            m_undone.clear();
            m_done.push_back(std::move(command));

            if (m_max_depth > 0 && m_done.size() > m_max_depth)
                m_done.erase(m_done.begin());
        }

        void undo() {
            if (m_done.empty())
                return;

            if (!m_done.back()->is_valid()) {
                clear();
                return;
            }

            m_done.back()->undo();
            m_undone.push_back(std::move(m_done.back()));
            m_done.pop_back();
        }

        void redo() {
            if (m_undone.empty())
                return;

            if (!m_undone.back()->is_valid()) {
                clear();
                return;
            }

            m_undone.back()->execute();
            m_done.push_back(std::move(m_undone.back()));
            m_undone.pop_back();
        }

        void clear() {
            m_done.clear();
            m_undone.clear();
        }

        [[nodiscard]] bool can_undo() const noexcept {
            return !m_done.empty();
        }

        [[nodiscard]] bool can_redo() const noexcept {
            return !m_undone.empty();
        }

        [[nodiscard]] std::string_view undo_label() const {
            return m_done.empty() ? std::string_view{} : m_done.back()->label();
        }

        [[nodiscard]] std::string_view redo_label() const {
            return m_undone.empty() ? std::string_view{} : m_undone.back()->label();
        }

        [[nodiscard]] std::size_t depth() const noexcept {
            return m_done.size();
        }

      private:
        std::vector<std::unique_ptr<ICommand>> m_done;
        std::vector<std::unique_ptr<ICommand>> m_undone;
        std::size_t m_max_depth;
    };
} // namespace star::editor
