#pragma once

#include <string_view>

namespace star::editor {
    class ICommand {
      public:
        virtual ~ICommand() = default;

        ICommand() = default;
        ICommand(const ICommand&) = delete;
        ICommand& operator=(const ICommand&) = delete;

        virtual void execute() = 0;
        virtual void undo() = 0;

        [[nodiscard]] virtual std::string_view label() const = 0;
        [[nodiscard]] virtual bool is_valid() const = 0;
    };
} // namespace star::editor
