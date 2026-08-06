#include "star/resources/import/async_model_loader.hpp"

#include "star/core/job_system.hpp"
#include "star/core/logger.hpp"
#include "star/resources/resource_manager.hpp"

namespace star::resources {
    AsyncModelLoader::AsyncModelLoader(JobSystem& jobs, ResourceManager& resources)
        : m_jobs(jobs), m_resources(resources) {}

    AsyncModelLoader::~AsyncModelLoader() {
        m_jobs.wait_idle();
    }

    u64 AsyncModelLoader::enqueue(std::filesystem::path path) {
        auto request = std::make_shared<Request>();
        request->path = std::move(path);
        request->display_name = request->path.filename().string();

        {
            std::lock_guard lock(m_mutex);
            request->id = m_next_id++;
            m_requests.push_back(request);
        }

        STAR_LOG_INFO(LogCategory::Resources, "Queued model import: {}", request->display_name);

        m_jobs.dispatch([request] {
            RawModel raw = parse_gltf(request->path);
            const bool ok = raw.ok;
            request->raw = std::move(raw);
            request->state.store(ok ? State::Parsed : State::Failed, std::memory_order_release);
        });

        return request->id;
    }

    std::vector<AsyncModelLoader::Completed> AsyncModelLoader::poll() {
        std::vector<std::shared_ptr<Request>> ready;

        {
            std::lock_guard lock(m_mutex);
            std::erase_if(m_requests, [&ready](const std::shared_ptr<Request>& request) {
                if (request->state.load(std::memory_order_acquire) == State::Parsing) {
                    return false;
                }
                ready.push_back(request);
                return true;
            });
        }

        std::vector<Completed> completed;
        completed.reserve(ready.size());

        for (const auto& request : ready) {
            if (request->state.load(std::memory_order_acquire) == State::Failed) {
                STAR_LOG_ERROR(LogCategory::Resources, "Model import failed: {}", request->display_name);
                continue;
            }

            Completed entry;
            entry.id = request->id;
            entry.path = request->path;
            entry.model = upload_gltf(m_resources, request->raw);

            if (!entry.model.valid()) {
                STAR_LOG_WARN(LogCategory::Resources, "Model '{}' produced no nodes", request->display_name);
                continue;
            }

            STAR_LOG_INFO(LogCategory::Resources, "Model import ready: {}", request->display_name);
            completed.push_back(std::move(entry));
        }

        return completed;
    }

    u32 AsyncModelLoader::in_flight() const {
        std::lock_guard lock(m_mutex);
        return static_cast<u32>(m_requests.size());
    }

    std::vector<std::string> AsyncModelLoader::in_flight_names() const {
        std::lock_guard lock(m_mutex);

        std::vector<std::string> names;
        names.reserve(m_requests.size());
        for (const auto& request : m_requests) {
            names.push_back(request->display_name);
        }
        return names;
    }
} // namespace star::resources
