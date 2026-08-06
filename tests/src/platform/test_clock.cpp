#include <thread>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "star/platform/time/clock.hpp"

using namespace star;

using namespace star::platform;

TEST_CASE("Clock - elapsed is near zero immediately after construction", "[platform][clock]") {
    const Clock clk;
    REQUIRE(clk.elapsed() < 1.0f);
    REQUIRE(clk.elapsed_millis() < 1000.0);
    REQUIRE(clk.elapsed_micros() < 1'000'000ull);
}

TEST_CASE("Clock - elapsed increases over time", "[platform][clock]") {
    const Clock clk;
    const f32 t1 = clk.elapsed();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    const f32 t2 = clk.elapsed();
    REQUIRE(t2 > t1);
}

TEST_CASE("Clock - elapsed_millis is 1000x elapsed in seconds", "[platform][clock]") {
    const Clock clk;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    const f32 secs = clk.elapsed();
    const f64 millis = clk.elapsed_millis();
    // millis should be approximately secs * 1000, within 10 ms tolerance
    REQUIRE(millis == Catch::Approx(static_cast<double>(secs) * 1000.0).margin(10.0));
}

TEST_CASE("Clock - reset restarts the timer", "[platform][clock]") {
    Clock clk;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    const f64 before = clk.elapsed_millis();
    clk.reset();
    const f64 after = clk.elapsed_millis();
    REQUIRE(after < before);
}

TEST_CASE("Clock - now() returns a valid time point", "[platform][clock]") {
    auto t1 = Clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto t2 = Clock::now();
    REQUIRE(t2 > t1);
}

TEST_CASE("FrameTimer - tick returns non-negative delta", "[platform][frame_timer]") {
    FrameTimer timer;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    const f32 dt = timer.tick();
    REQUIRE(dt >= 0.0f);
}

TEST_CASE("FrameTimer - delta_time matches tick return value", "[platform][frame_timer]") {
    FrameTimer timer;
    const f32 returned = timer.tick();
    REQUIRE(timer.delta_time() == returned);
}

TEST_CASE("FrameTimer - frame_count increments on each tick", "[platform][frame_timer]") {
    FrameTimer timer;
    timer.tick();
    REQUIRE(timer.frame_count() == 1u);
    timer.tick();
    REQUIRE(timer.frame_count() == 2u);
    timer.tick();
    REQUIRE(timer.frame_count() == 3u);
}

TEST_CASE("FrameTimer - total_time increases after a delay", "[platform][frame_timer]") {
    FrameTimer timer;
    timer.tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    timer.tick();
    REQUIRE(timer.total_time() > 0.0f);
}

TEST_CASE("FrameTimer - reset clears accumulated state", "[platform][frame_timer]") {
    FrameTimer timer;
    timer.tick();
    timer.tick();
    timer.reset();
    REQUIRE(timer.frame_count() == 0u);
    REQUIRE(timer.total_time() == Catch::Approx(0.0f));
}

TEST_CASE("FrameTimer - set_target_fps stores the value", "[platform][frame_timer]") {
    FrameTimer timer;
    timer.set_target_fps(60u);
    REQUIRE(timer.target_fps() == 60u);
}

TEST_CASE("FrameTimer - target_fps zero means uncapped", "[platform][frame_timer]") {
    const FrameTimer timer(0u);
    REQUIRE(timer.target_fps() == 0u);
}
