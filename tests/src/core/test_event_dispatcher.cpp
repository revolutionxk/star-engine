#include <catch2/catch_test_macros.hpp>

#include "star/core/event_dispatcher.hpp"

using namespace star;

struct TestEvent {
    int value{0};
};

TEST_CASE("EventListenerHandle - validity", "[core][events]") {
    constexpr EventListenerHandle invalid{};
    constexpr EventListenerHandle valid{1};

    REQUIRE_FALSE(invalid.is_valid());
    REQUIRE(valid.is_valid());
}

TEST_CASE("EventDispatcher - starts empty", "[core][events]") {
    const EventDispatcher<TestEvent> dispatcher;
    REQUIRE(dispatcher.empty());
}

TEST_CASE("EventDispatcher - add returns valid handle", "[core][events]") {
    EventDispatcher<TestEvent> dispatcher;
    const auto handle = dispatcher.add([](const TestEvent&) { return false; });
    REQUIRE(handle.is_valid());
    REQUIRE_FALSE(dispatcher.empty());
}

TEST_CASE("EventDispatcher - dispatch calls handler", "[core][events]") {
    EventDispatcher<TestEvent> dispatcher;
    int received = -1;

    dispatcher.add([&](const TestEvent& e) {
        received = e.value;
        return false;
    });

    dispatcher.dispatch({42});
    REQUIRE(received == 42);
}

TEST_CASE("EventDispatcher - returning true stops propagation", "[core][events]") {
    EventDispatcher<TestEvent> dispatcher;
    int call_count = 0;

    dispatcher.add(
        [&](const TestEvent&) {
            ++call_count;
            return true;
        },
        10);
    dispatcher.add(
        [&](const TestEvent&) {
            ++call_count;
            return false;
        },
        0);

    const auto consumed = dispatcher.dispatch({});
    REQUIRE(consumed);
    REQUIRE(call_count == 1);
}

TEST_CASE("EventDispatcher - returning false allows full propagation", "[core][events]") {
    EventDispatcher<TestEvent> dispatcher;
    int call_count = 0;

    dispatcher.add([&](const TestEvent&) {
        ++call_count;
        return false;
    });
    dispatcher.add([&](const TestEvent&) {
        ++call_count;
        return false;
    });

    const bool consumed = dispatcher.dispatch({});
    REQUIRE_FALSE(consumed);
    REQUIRE(call_count == 2);
}

TEST_CASE("EventDispatcher - handlers fire in descending priority order", "[core][events]") {
    EventDispatcher<TestEvent> dispatcher;
    std::vector<int> order;

    dispatcher.add(
        [&](const TestEvent&) {
            order.push_back(1);
            return false;
        },
        1);
    dispatcher.add(
        [&](const TestEvent&) {
            order.push_back(10);
            return false;
        },
        10);
    dispatcher.add(
        [&](const TestEvent&) {
            order.push_back(5);
            return false;
        },
        5);

    dispatcher.dispatch({0});

    REQUIRE(order == std::vector{10, 5, 1});
}

TEST_CASE("EventDispatcher - remove listener stops invocations", "[core][events]") {
    EventDispatcher<TestEvent> dispatcher;
    int received = 0;

    auto handle = dispatcher.add([&](const TestEvent& e) {
        received = e.value;
        return false;
    });

    dispatcher.remove(handle);
    REQUIRE(dispatcher.empty());

    dispatcher.dispatch({99});
    REQUIRE(received == 0);
}

TEST_CASE("EventDispatcher - removing nonexistent handle is harmless", "[core][events]") {
    EventDispatcher<TestEvent> dispatcher;
    constexpr EventListenerHandle ghost{9999};
    REQUIRE_NOTHROW(dispatcher.remove(ghost));
}

TEST_CASE("EventDispatcher - dispatch returns false when no handlers", "[core][events]") {
    const EventDispatcher<TestEvent> dispatcher;
    REQUIRE_FALSE(dispatcher.dispatch({1}));
}

TEST_CASE("VoidEventDispatcher - all handlers are called", "[core][events]") {
    VoidEventDispatcher<TestEvent> dispatcher;
    int total = 0;

    dispatcher.add([&](const TestEvent& e) { total += e.value; });
    dispatcher.add([&](const TestEvent& e) { total += e.value; });

    dispatcher.dispatch({10});
    REQUIRE(total == 20);
}

TEST_CASE("VoidEventDispatcher - remove handler stops calls", "[core][events]") {
    VoidEventDispatcher<TestEvent> dispatcher;
    int count = 0;

    const auto h = dispatcher.add([&](const TestEvent&) { ++count; });
    dispatcher.remove(h);
    dispatcher.dispatch({1});

    REQUIRE(count == 0);
    REQUIRE(dispatcher.empty());
}

TEST_CASE("VoidEventDispatcher - unique handle ids per listener", "[core][events]") {
    VoidEventDispatcher<TestEvent> dispatcher;
    const auto [id1] = dispatcher.add([](const TestEvent&) {});
    const auto [id2] = dispatcher.add([](const TestEvent&) {});
    REQUIRE(id1 != id2);
}
