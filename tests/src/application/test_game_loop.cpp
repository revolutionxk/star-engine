#include <thread>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "star/application/game_loop.hpp"

using namespace star::application;

TEST_CASE("GameLoopConfig - default values are sensible", "[application][game_loop]") {
    GameLoopConfig cfg;
    REQUIRE(cfg.fixed_timestep == Catch::Approx(1.0f / 60.0f).margin(1e-5f));
    REQUIRE(cfg.max_frame_time == Catch::Approx(0.1f));
    REQUIRE(cfg.use_fixed_timestep == false);
    REQUIRE(cfg.max_updates_per_frame == 5u);
    REQUIRE(cfg.target_fps == 0u);
}

TEST_CASE("GameLoop - default construction uses default config", "[application][game_loop]") {
    const GameLoop loop;
    REQUIRE(loop.config().fixed_timestep == Catch::Approx(1.0f / 60.0f).margin(1e-5f));
}

TEST_CASE("GameLoop - custom config is stored", "[application][game_loop]") {
    GameLoopConfig cfg;
    cfg.target_fps = 30u;
    cfg.max_updates_per_frame = 3u;

    const GameLoop loop(cfg);
    REQUIRE(loop.config().target_fps == 30u);
    REQUIRE(loop.config().max_updates_per_frame == 3u);
}

TEST_CASE("GameLoop - tick returns non-negative delta", "[application][game_loop]") {
    GameLoop loop;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    const f32 dt = loop.tick();
    REQUIRE(dt >= 0.0f);
    REQUIRE(loop.delta_time() == dt);
}

TEST_CASE("GameLoop - consecutive ticks accumulate time", "[application][game_loop]") {
    GameLoop loop;
    loop.tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    const f32 dt2 = loop.tick();
    REQUIRE(dt2 > 0.0f);
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
    cfg.use_fixed_timestep = true;
    loop.set_config(cfg);
    REQUIRE(loop.config().use_fixed_timestep == true);
    REQUIRE(loop.config().fixed_timestep == Catch::Approx(1.0f / 30.0f).margin(1e-5f));
}

TEST_CASE("GameLoop - run calls update and terminates when predicate is false", "[application][game_loop]") {
    GameLoop loop;
    int update_count = 0;

    loop.run([&](f32) { ++update_count; }, [] {}, [&] { return update_count < 3; });

    REQUIRE(update_count >= 3);
}

TEST_CASE("GameLoop - run never calls update when predicate starts false", "[application][game_loop]") {
    GameLoop loop;
    int count = 0;

    loop.run([&](f32) { ++count; }, [] {}, [] { return false; });

    REQUIRE(count == 0);
}
