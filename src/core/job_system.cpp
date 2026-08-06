#include "star/core/job_system.hpp"

#include <algorithm>

#include "star/core/logger.hpp"

namespace star {
    thread_local const JobSystem* t_current_system = nullptr;

    JobSystem::JobSystem(const u32 worker_count) : m_owner_thread(std::this_thread::get_id()) {
        u32 count = worker_count;
        if (count == 0) {
            const u32 hardware = std::thread::hardware_concurrency();
            count = hardware > 1 ? hardware - 1 : 1;
        }

        m_workers.reserve(count);
        for (u32 i = 0; i < count; ++i) {
            m_workers.push_back(std::make_unique<Worker>());
        }

        m_threads.reserve(count);
        for (u32 i = 0; i < count; ++i) {
            m_threads.emplace_back([this, i] { worker_loop(i); });
        }

        STAR_LOG_INFO(LogCategory::Core, "JobSystem started with {} worker threads", count);
    }

    JobSystem::~JobSystem() {
        wait_idle();

        m_running.store(false, std::memory_order_release);
        m_wake.notify_all();

        for (auto& thread : m_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        STAR_LOG_INFO(LogCategory::Core, "JobSystem stopped");
    }

    bool JobSystem::on_worker_thread() const noexcept {
        return t_current_system == this;
    }

    void JobSystem::dispatch(Job job) {
        if (!job) {
            return;
        }

        if (m_workers.empty()) {
            job();
            return;
        }

        const u32 index = m_round_robin.fetch_add(1, std::memory_order_relaxed) % worker_count();

        m_pending.fetch_add(1, std::memory_order_acq_rel);
        {
            std::lock_guard lock(m_workers[index]->mutex);
            m_workers[index]->queue.push_back(std::move(job));
        }
        m_wake.notify_one();
    }

    bool JobSystem::try_pop_local(const u32 index, Job& out) {
        Worker& worker = *m_workers[index];
        std::lock_guard lock(worker.mutex);
        if (worker.queue.empty()) {
            return false;
        }
        out = std::move(worker.queue.back());
        worker.queue.pop_back();
        return true;
    }

    bool JobSystem::try_steal(const u32 thief, Job& out) {
        const u32 count = worker_count();
        for (u32 offset = 1; offset < count; ++offset) {
            Worker& victim = *m_workers[(thief + offset) % count];
            std::lock_guard lock(victim.mutex);
            if (victim.queue.empty()) {
                continue;
            }
            out = std::move(victim.queue.front());
            victim.queue.pop_front();
            return true;
        }
        return false;
    }

    bool JobSystem::try_get_any(const u32 preferred, Job& out) {
        return try_pop_local(preferred, out) || try_steal(preferred, out);
    }

    void JobSystem::run(Job& job) {
        job();
        job = nullptr;

        if (m_pending.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            std::lock_guard lock(m_wake_mutex);
            m_idle.notify_all();
        }
    }

    void JobSystem::worker_loop(const u32 index) {
        t_current_system = this;

        while (m_running.load(std::memory_order_acquire)) {
            Job job;
            if (try_get_any(index, job)) {
                run(job);
                continue;
            }

            std::unique_lock lock(m_wake_mutex);
            m_wake.wait_for(lock, std::chrono::milliseconds(2), [this] {
                return !m_running.load(std::memory_order_acquire) ||
                       m_pending.load(std::memory_order_acquire) > 0;
            });
        }

        t_current_system = nullptr;
    }

    void JobSystem::wait_idle() {
        const u32 helper = m_round_robin.load(std::memory_order_relaxed) % std::max(1u, worker_count());

        while (m_pending.load(std::memory_order_acquire) > 0) {
            Job job;
            if (!m_workers.empty() && try_get_any(helper, job)) {
                run(job);
                continue;
            }

            std::unique_lock lock(m_wake_mutex);
            m_idle.wait_for(lock, std::chrono::milliseconds(1),
                            [this] { return m_pending.load(std::memory_order_acquire) == 0; });
        }
    }

    void JobSystem::parallel_for(const u32 count, const u32 grain, const IndexedJob& body) {
        if (count == 0 || !body) {
            return;
        }

        const u32 step = std::max(1u, grain);

        if (m_workers.empty() || on_worker_thread() || count <= step) {
            body(0, count);
            return;
        }

        const u32 chunks = (count + step - 1) / step;

        std::atomic<u32> remaining{chunks};

        for (u32 chunk = 1; chunk < chunks; ++chunk) {
            const u32 begin = chunk * step;
            const u32 end = std::min(begin + step, count);
            dispatch([&body, &remaining, begin, end] {
                body(begin, end);
                remaining.fetch_sub(1, std::memory_order_acq_rel);
            });
        }

        body(0, std::min(step, count));
        remaining.fetch_sub(1, std::memory_order_acq_rel);

        const u32 helper = m_round_robin.load(std::memory_order_relaxed) % worker_count();
        while (remaining.load(std::memory_order_acquire) > 0) {
            Job job;
            if (try_get_any(helper, job)) {
                run(job);
                continue;
            }
            std::this_thread::yield();
        }
    }
} // namespace star
