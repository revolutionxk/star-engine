#include <memory>
#include <string>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

#include "core/commands/command_stack.hpp"

using namespace star::editor;

namespace {
    class CounterCommand final : public ICommand {
      public:
        CounterCommand(int& value, const int delta, const bool valid = true, std::string label = "Counter")
            : m_value(value), m_delta(delta), m_label(std::move(label)), m_valid(valid) {}

        void execute() override {
            m_value += m_delta;
        }

        void undo() override {
            m_value -= m_delta;
        }

        [[nodiscard]] std::string_view label() const override {
            return m_label;
        }

        [[nodiscard]] bool is_valid() const override {
            return m_valid;
        }

        void set_valid(const bool valid) {
            m_valid = valid;
        }

      private:
        int& m_value;
        int m_delta;
        std::string m_label;
        bool m_valid;
    };
} // namespace

TEST_CASE("push executes the command", "[editor][commands]") {
    int value = 0;
    CommandStack stack;
    stack.push(std::make_unique<CounterCommand>(value, 5));
    REQUIRE(value == 5);
    REQUIRE(stack.can_undo());
    REQUIRE_FALSE(stack.can_redo());
}

TEST_CASE("undo and redo move between the stacks", "[editor][commands]") {
    int value = 0;
    CommandStack stack;
    stack.push(std::make_unique<CounterCommand>(value, 5));

    stack.undo();
    REQUIRE(value == 0);
    REQUIRE_FALSE(stack.can_undo());
    REQUIRE(stack.can_redo());

    stack.redo();
    REQUIRE(value == 5);
    REQUIRE(stack.can_undo());
    REQUIRE_FALSE(stack.can_redo());
}

TEST_CASE("push clears the redo stack", "[editor][commands]") {
    int value = 0;
    CommandStack stack;
    stack.push(std::make_unique<CounterCommand>(value, 5));
    stack.undo();
    REQUIRE(stack.can_redo());

    stack.push(std::make_unique<CounterCommand>(value, 3));
    REQUIRE_FALSE(stack.can_redo());
    REQUIRE(value == 3);
}

TEST_CASE("the stack drops the oldest command past its depth", "[editor][commands]") {
    int value = 0;
    CommandStack stack{2};
    stack.push(std::make_unique<CounterCommand>(value, 1, true, "First"));
    stack.push(std::make_unique<CounterCommand>(value, 10, true, "Second"));
    stack.push(std::make_unique<CounterCommand>(value, 100, true, "Third"));
    REQUIRE(stack.depth() == 2);
    REQUIRE(value == 111);

    REQUIRE(stack.undo_label() == "Third");
    stack.undo();
    REQUIRE(value == 11);

    REQUIRE(stack.undo_label() == "Second");
    stack.undo();
    REQUIRE(value == 1);

    REQUIRE_FALSE(stack.can_undo());
}

TEST_CASE("clear empties both stacks", "[editor][commands]") {
    int value = 0;
    CommandStack stack;
    stack.push(std::make_unique<CounterCommand>(value, 1));
    stack.undo();
    stack.clear();
    REQUIRE_FALSE(stack.can_undo());
    REQUIRE_FALSE(stack.can_redo());
}

TEST_CASE("an invalid command clears the stack instead of applying", "[editor][commands]") {
    int value = 0;
    CommandStack stack;
    stack.push(std::make_unique<CounterCommand>(value, 5, false));
    REQUIRE(value == 5);

    stack.undo();
    REQUIRE(value == 5);
    REQUIRE_FALSE(stack.can_undo());
    REQUIRE_FALSE(stack.can_redo());
}

TEST_CASE("labels follow the top of each stack", "[editor][commands]") {
    int value = 0;
    CommandStack stack;
    REQUIRE(stack.undo_label().empty());

    stack.push(std::make_unique<CounterCommand>(value, 1));
    REQUIRE(stack.undo_label() == "Counter");
    REQUIRE(stack.redo_label().empty());

    stack.undo();
    REQUIRE(stack.redo_label() == "Counter");
}

TEST_CASE("an invalid command clears the stack instead of redoing", "[editor][commands]") {
    int value = 0;
    CommandStack stack;

    auto command = std::make_unique<CounterCommand>(value, 5);
    CounterCommand* raw = command.get();
    stack.push(std::move(command));
    stack.undo();
    REQUIRE(value == 0);
    REQUIRE(stack.can_redo());

    raw->set_valid(false);
    stack.redo();

    REQUIRE(value == 0);
    REQUIRE_FALSE(stack.can_undo());
    REQUIRE_FALSE(stack.can_redo());
}
