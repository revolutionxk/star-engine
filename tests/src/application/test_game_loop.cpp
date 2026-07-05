#include <chrono>
#include <thread>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "star/application/game_loop.hpp"

using namespace star::application;

TEST_CASE("GameLoopConfig - default values are sensible", "[application][game_loop]") {
    GameLoopConfig cfg;
    REQUIRE(cfg.fixed_timestep == Catch::Approx(1.0f / 60.0f).margin(1e-5f));
    REQUIRE(cfg.max_frame_time == Catch::Approx(0.25f));
    REQUIRE(cfg.max_fixed_steps == 5u);
    REQUIRE(cfg.target_fps == 0u);
}

TEST_CASE("GameLoop - default construction uses default config", "[application][game_loop]") {
    const GameLoop loop;
    REQUIRE(loop.config().fixed_timestep == Catch::Approx(1.0f / 60.0f).margin(1e-5f));
}

TEST_CASE("GameLoop - custom config is stored", "[application][game_loop]") {
    GameLoopConfig cfg;
    cfg.target_fps = 30u;
    cfg.max_fixed_steps = 3u;

    const GameLoop loop(cfg);
    REQUIRE(loop.config().target_fps == 30u);
    REQUIRE(loop.config().max_fixed_steps == 3u);
}

TEST_CASE("GameLoop - tick returns non-negative delta", "[application][game_loop]") {
    GameLoop loop;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    const f32 dt = loop.tick();
    REQUIRE(dt >= 0.0f);
    REQUIRE(loop.delta_time() == dt);
}

TEST_CASE("GameLoop - set_target_fps updates config", "[application][game_loop]") {
    GameLoop loop;
    loop.set_target_fps(60u);
    REQUIRE(loop.config().target_fps == 60u);
}

TEST_CASE("GameLoop - set_config replaces full config", "[application][game_loop]") {
    GameLoop loop;
    GameLoopConfig cfg;
    cfg.fixed_timestep = 1.0f / 30.0f;
    cfg.max_fixed_steps = 8u;
    loop.set_config(cfg);
    REQUIRE(loop.config().max_fixed_steps == 8u);
    REQUIRE(loop.config().fixed_timestep == Catch::Approx(1.0f / 30.0f).margin(1e-5f));
}

TEST_CASE("GameLoop - run calls update and terminates when predicate is false", "[application][game_loop]") {
    GameLoop loop;
    int update_count = 0;

    loop.run([](f32) {}, [&](f32) { ++update_count; }, [](f32) {}, [&] { return update_count < 3; });

    REQUIRE(update_count >= 3);
}

TEST_CASE("GameLoop - run never calls update when predicate starts false", "[application][game_loop]") {
    GameLoop loop;
    int count = 0;

    loop.run([](f32) {}, [&](f32) { ++count; }, [](f32) {}, [] { return false; });

    REQUIRE(count == 0);
}

TEST_CASE("GameLoop - render receives an interpolation alpha in [0, 1)", "[application][game_loop]") {
    GameLoop loop;
    f32 alpha = -1.0f;
    int frames = 0;

    loop.run([](f32) {}, [](f32) {}, [&](const f32 a) { alpha = a; }, [&] { return frames++ < 3; });

    REQUIRE(alpha >= 0.0f);
    REQUIRE(alpha < 1.0f);
}

TEST_CASE("GameLoop - fixed_update steps once enough time has accumulated", "[application][game_loop]") {
    GameLoopConfig cfg;
    cfg.fixed_timestep = 1.0f / 1000.0f;
    GameLoop loop(cfg);

    int fixed_count = 0;
    int frames = 0;

    loop.run([&](f32) { ++fixed_count; }, [](f32) { std::this_thread::sleep_for(std::chrono::milliseconds(3)); },
             [](f32) {}, [&] { return frames++ < 3; });

    REQUIRE(fixed_count >= 1);
}
