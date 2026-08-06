#pragma once

#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "star/core/types.hpp"
#include "star/resources/import/gltf_importer.hpp"

namespace star {
    class JobSystem;
} // namespace star

namespace star::resources {
    class ResourceManager;

    class AsyncModelLoader {
      public:
        AsyncModelLoader(JobSystem& jobs, ResourceManager& resources);
        ~AsyncModelLoader();

        AsyncModelLoader(const AsyncModelLoader&) = delete;
        AsyncModelLoader& operator=(const AsyncModelLoader&) = delete;

        struct Completed {
            u64 id{0};
            std::filesystem::path path;
            ImportedModel model;
        };

        u64 enqueue(std::filesystem::path path);

        [[nodiscard]] std::vector<Completed> poll();

        [[nodiscard]] u32 in_flight() const;
        [[nodiscard]] std::vector<std::string> in_flight_names() const;

      private:
        enum class State : u8 {
            Parsing,
            Parsed,
            Failed,
        };

        struct Request {
            u64 id{0};
            std::filesystem::path path;
            std::string display_name;
            std::atomic<State> state{State::Parsing};
            RawModel raw;
        };

        JobSystem& m_jobs;
        ResourceManager& m_resources;

        mutable std::mutex m_mutex;
        std::vector<std::shared_ptr<Request>> m_requests;
        u64 m_next_id{1};
    };
} // namespace star::resources
