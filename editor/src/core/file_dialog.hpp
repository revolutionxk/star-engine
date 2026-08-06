#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include "star/core/types.hpp"
#include "star/platform/file_dialog.hpp"

namespace star::editor {
    enum class FileRequest : u32 {
        Model,
        Environment,
        Scene,
    };

    class EditorFileDialogs {
      public:
        EditorFileDialogs();
        ~EditorFileDialogs();

        EditorFileDialogs(const EditorFileDialogs&) = delete;
        EditorFileDialogs& operator=(const EditorFileDialogs&) = delete;

        struct Result {
            FileRequest request{FileRequest::Model};
            std::filesystem::path path;
        };

        void open(FileRequest request, const platform::Window* owner);

        [[nodiscard]] std::vector<Result> poll() const;

        [[nodiscard]] bool is_open() const;

      private:
        std::unique_ptr<platform::FileDialog> m_dialog;
    };
} // namespace star::editor
