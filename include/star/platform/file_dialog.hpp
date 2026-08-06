#pragma once

#include <filesystem>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include "star/core/types.hpp"

namespace star::platform {
    class Window;

    struct FileFilter {
        std::string_view label;
        std::string_view extensions;
    };

    class FileDialog {
      public:
        struct Result {
            u32 tag{0};
            std::filesystem::path path;
        };

        virtual ~FileDialog() = default;

        FileDialog(const FileDialog&) = delete;
        FileDialog& operator=(const FileDialog&) = delete;

        virtual void open(u32 tag, std::span<const FileFilter> filters, const Window* owner) = 0;

        [[nodiscard]] virtual std::vector<Result> poll() = 0;

        [[nodiscard]] virtual bool is_open() const = 0;

        static std::unique_ptr<FileDialog> create();

      protected:
        FileDialog() = default;
    };
} // namespace star::platform
