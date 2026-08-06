#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "star/core/types.hpp"

namespace star {
    class JobSystem {
      public:
        using Job = std::function<void()>;
        using IndexedJob = std::function<void(u32 begin, u32 end)>;

        explicit JobSystem(u32 worker_count = 0);
        ~JobSystem();

        JobSystem(const JobSystem&) = delete;
        JobSystem& operator=(const JobSystem&) = delete;

        void dispatch(Job job);

        void parallel_for(u32 count, u32 grain, const IndexedJob& body);

        void wait_idle();

        [[nodiscard]] u32 worker_count() const noexcept {
            return static_cast<u32>(m_workers.size());
        }

        [[nodiscard]] bool on_worker_thread() const noexcept;

      private:
        struct Worker {
            std::mutex mutex;
            std::deque<Job> queue;
        };

        void worker_loop(u32 index);
        bool try_pop_local(u32 index, Job& out);
        bool try_steal(u32 thief, Job& out);
        bool try_get_any(u32 preferred, Job& out);
        void run(Job& job);

        std::vector<std::unique_ptr<Worker>> m_workers;
        std::vector<std::thread> m_threads;

        std::mutex m_wake_mutex;
        std::condition_variable m_wake;
        std::condition_variable m_idle;

        std::atomic<u32> m_pending{0};
        std::atomic<u32> m_round_robin{0};
        std::atomic<bool> m_running{true};

        std::thread::id m_owner_thread;
    };
} // namespace star
