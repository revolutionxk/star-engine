#include "file_dialog.hpp"

#include <array>

#include "star/core/logger.hpp"

namespace star::editor {
    constexpr std::array MODEL_FILTERS{
        platform::FileFilter{"glTF models", "gltf;glb"},
        platform::FileFilter{"All files", "*"},
    };

    constexpr std::array ENVIRONMENT_FILTERS{
        platform::FileFilter{"HDR images", "hdr;exr"},
        platform::FileFilter{"All files", "*"},
    };

    constexpr std::array SCENE_FILTERS{
        platform::FileFilter{"Star scenes", "scene;json"},
        platform::FileFilter{"All files", "*"},
    };

    std::span<const platform::FileFilter> filters_for(const FileRequest request) {
        switch (request) {
            case FileRequest::Environment:
                return ENVIRONMENT_FILTERS;
            case FileRequest::Scene:
                return SCENE_FILTERS;
            case FileRequest::Model:
            default:
                return MODEL_FILTERS;
        }
    }

    EditorFileDialogs::EditorFileDialogs() : m_dialog(platform::FileDialog::create()) {}

    EditorFileDialogs::~EditorFileDialogs() = default;

    void EditorFileDialogs::open(const FileRequest request, const platform::Window* owner) {
        if (!m_dialog) {
            STAR_LOG_ERROR(LogCategory::Editor, "No file dialog implementation available");
            return;
        }
        m_dialog->open(static_cast<u32>(request), filters_for(request), owner);
    }

    std::vector<EditorFileDialogs::Result> EditorFileDialogs::poll() const {
        if (!m_dialog) {
            return {};
        }

        std::vector<Result> results;
        for (auto& [tag, path] : m_dialog->poll()) {
            results.push_back({static_cast<FileRequest>(tag), std::move(path)});
        }
        return results;
    }

    bool EditorFileDialogs::is_open() const {
        return m_dialog && m_dialog->is_open();
    }
} // namespace star::editor
