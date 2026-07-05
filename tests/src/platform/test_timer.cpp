#include <catch2/catch_test_macros.hpp>

#include "star/platform/time/timer.hpp"

using namespace star::platform;

TEST_CASE("Countdown - fires exactly once on the frame it finishes", "[platform][timer]") {
    Countdown countdown(0.0f, true);

    REQUIRE(countdown.is_running());
    REQUIRE(countdown.update(0.016f));
    REQUIRE_FALSE(countdown.update(0.016f));
    REQUIRE(countdown.is_finished());
    REQUIRE_FALSE(countdown.is_running());
}

TEST_CASE("Countdown - does not fire while still running", "[platform][timer]") {
    Countdown countdown(100.0f, true);

    REQUIRE_FALSE(countdown.update(0.016f));
    REQUIRE(countdown.is_running());
    REQUIRE_FALSE(countdown.is_finished());
}

TEST_CASE("Countdown - a stopped countdown never fires", "[platform][timer]") {
    Countdown countdown(0.0f, false);

    REQUIRE_FALSE(countdown.is_running());
    REQUIRE_FALSE(countdown.update(0.016f));
}
