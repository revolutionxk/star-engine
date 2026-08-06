#include <atomic>
#include <numeric>
#include <set>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "star/core/job_system.hpp"

using namespace star;

TEST_CASE("JobSystem spawns workers", "[core][jobs]") {
    const JobSystem jobs(4);
    REQUIRE(jobs.worker_count() == 4);
}

TEST_CASE("JobSystem falls back to inline execution with zero workers", "[core][jobs]") {
    JobSystem jobs(0);
    REQUIRE(jobs.worker_count() >= 1);
}

TEST_CASE("dispatch runs every job exactly once", "[core][jobs]") {
    JobSystem jobs(4);

    constexpr u32 job_count = 2000;
    std::atomic<u32> counter{0};

    for (u32 i = 0; i < job_count; ++i) {
        jobs.dispatch([&counter] { counter.fetch_add(1, std::memory_order_relaxed); });
    }

    jobs.wait_idle();
    REQUIRE(counter.load() == job_count);
}

TEST_CASE("wait_idle drains jobs queued from the calling thread", "[core][jobs]") {
    JobSystem jobs(2);

    std::atomic<u64> sum{0};
    for (u32 i = 1; i <= 100; ++i) {
        jobs.dispatch([&sum, i] { sum.fetch_add(i, std::memory_order_relaxed); });
    }

    jobs.wait_idle();
    REQUIRE(sum.load() == 5050);
}

TEST_CASE("parallel_for covers the whole range without gaps or overlap", "[core][jobs]") {
    JobSystem jobs(4);

    constexpr u32 count = 10000;
    std::vector<u32> touched(count, 0);

    jobs.parallel_for(count, 64, [&touched](const u32 begin, const u32 end) {
        for (u32 i = begin; i < end; ++i) {
            ++touched[i];
        }
    });

    REQUIRE(std::ranges::all_of(touched, [](const u32 v) { return v == 1; }));
}

TEST_CASE("parallel_for handles ranges smaller than the grain", "[core][jobs]") {
    JobSystem jobs(4);

    std::vector<u32> touched(5, 0);
    jobs.parallel_for(5, 64, [&touched](const u32 begin, const u32 end) {
        for (u32 i = begin; i < end; ++i) {
            ++touched[i];
        }
    });

    REQUIRE(std::ranges::all_of(touched, [](const u32 v) { return v == 1; }));
}

TEST_CASE("parallel_for with zero count is a no-op", "[core][jobs]") {
    JobSystem jobs(2);

    bool called = false;
    jobs.parallel_for(0, 16, [&called](u32, u32) { called = true; });

    REQUIRE_FALSE(called);
}

TEST_CASE("parallel_for computes a correct reduction", "[core][jobs]") {
    JobSystem jobs(4);

    constexpr u32 count = 50000;
    std::vector<u64> values(count);
    std::iota(values.begin(), values.end(), 1ull);

    std::atomic<u64> total{0};
    jobs.parallel_for(count, 256, [&](const u32 begin, const u32 end) {
        u64 local = 0;
        for (u32 i = begin; i < end; ++i) {
            local += values[i];
        }
        total.fetch_add(local, std::memory_order_relaxed);
    });

    constexpr u64 expected = static_cast<u64>(count) * (count + 1) / 2;
    REQUIRE(total.load() == expected);
}

TEST_CASE("nested parallel_for runs inline instead of deadlocking", "[core][jobs]") {
    JobSystem jobs(2);

    std::atomic<u32> inner_total{0};

    jobs.parallel_for(8, 1, [&](const u32 begin, const u32 end) {
        for (u32 i = begin; i < end; ++i) {
            jobs.parallel_for(4, 1, [&](const u32 b, const u32 e) {
                inner_total.fetch_add(e - b, std::memory_order_relaxed);
            });
        }
    });

    REQUIRE(inner_total.load() == 8 * 4);
}

TEST_CASE("destructor waits for pending work", "[core][jobs]") {
    std::atomic<u32> counter{0};
    {
        JobSystem jobs(4);
        for (u32 i = 0; i < 500; ++i) {
            jobs.dispatch([&counter] { counter.fetch_add(1, std::memory_order_relaxed); });
        }
    }
    REQUIRE(counter.load() == 500);
}

TEST_CASE("jobs dispatched from inside a job still complete", "[core][jobs]") {
    JobSystem jobs(4);

    std::atomic<u32> counter{0};
    for (u32 i = 0; i < 50; ++i) {
        jobs.dispatch([&] {
            counter.fetch_add(1, std::memory_order_relaxed);
            jobs.dispatch([&] { counter.fetch_add(1, std::memory_order_relaxed); });
        });
    }

    jobs.wait_idle();
    REQUIRE(counter.load() == 100);
}
