#include "sdl_file_dialog.hpp"

#include <memory>
#include <utility>

#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_error.h>

#include "star/core/common.hpp"
#include "star/platform/window.hpp"

namespace star::platform::sdl {
    SDLFileDialog::~SDLFileDialog() {
        if (is_open()) {
            STAR_LOG_WARN(LogCategory::Platform, "File dialog destroyed while still open");
        }
    }

    void SDLFileDialog::open(const u32 tag, const std::span<const FileFilter> filters, const Window* owner) {
        auto request = std::make_unique<Request>();
        request->dialog = this;
        request->tag = tag;
        request->labels.reserve(filters.size());
        request->extensions.reserve(filters.size());

        for (const auto& [label, extensions] : filters) {
            request->labels.emplace_back(label);
            request->extensions.emplace_back(extensions);
        }

        std::vector<SDL_DialogFileFilter> sdl_filters;
        sdl_filters.reserve(filters.size());
        for (std::size_t i = 0; i < filters.size(); ++i) {
            sdl_filters.push_back({request->labels[i].c_str(), request->extensions[i].c_str()});
        }

        {
            std::lock_guard lock(m_mutex);
            ++m_open_count;
        }

        auto* window = owner ? static_cast<SDL_Window*>(owner->handle()) : nullptr;
        SDL_ShowOpenFileDialog(&SDLFileDialog::on_selected, request.release(), window, sdl_filters.data(),
                               static_cast<int>(sdl_filters.size()), nullptr, false);
    }

    void SDLFileDialog::on_selected(void* userdata, const char* const* filelist, int /*filter*/) {
        const std::unique_ptr<Request> request{static_cast<Request*>(userdata)};
        if (!request || !request->dialog) {
            return;
        }

        if (!filelist) {
            STAR_LOG_ERROR(LogCategory::Platform, "File dialog failed: {}", SDL_GetError());
            request->dialog->complete(request->tag, {});
            return;
        }

        if (!filelist[0]) {
            request->dialog->complete(request->tag, {});
            return;
        }

        request->dialog->complete(request->tag, std::filesystem::path{filelist[0]});
    }

    void SDLFileDialog::complete(const u32 tag, std::filesystem::path path) {
        std::lock_guard lock(m_mutex);
        if (m_open_count > 0) {
            --m_open_count;
        }
        if (!path.empty()) {
            m_results.push_back({tag, std::move(path)});
        }
    }

    std::vector<FileDialog::Result> SDLFileDialog::poll() {
        std::lock_guard lock(m_mutex);
        return std::exchange(m_results, {});
    }

    bool SDLFileDialog::is_open() const {
        std::lock_guard lock(m_mutex);
        return m_open_count > 0;
    }
} // namespace star::platform::sdl
