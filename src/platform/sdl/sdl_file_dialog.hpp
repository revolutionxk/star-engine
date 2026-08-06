#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "star/platform/file_dialog.hpp"

namespace star::platform::sdl {
    class SDLFileDialog final : public FileDialog {
      public:
        SDLFileDialog() = default;
        ~SDLFileDialog() override;

        void open(u32 tag, std::span<const FileFilter> filters, const Window* owner) override;

        [[nodiscard]] std::vector<Result> poll() override;

        [[nodiscard]] bool is_open() const override;

      private:
        struct Request {
            SDLFileDialog* dialog{nullptr};
            u32 tag{0};
            std::vector<std::string> labels;
            std::vector<std::string> extensions;
        };

        static void on_selected(void* userdata, const char* const* filelist, int filter);

        void complete(u32 tag, std::filesystem::path path);

        mutable std::mutex m_mutex;
        std::vector<Result> m_results;
        u32 m_open_count{0};
    };
} // namespace star::platform::sdl
