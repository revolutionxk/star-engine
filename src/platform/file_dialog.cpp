#include "star/platform/file_dialog.hpp"

#include "sdl/sdl_file_dialog.hpp"
#include "star/core/common.hpp"

namespace star::platform {
    std::unique_ptr<FileDialog> FileDialog::create() {
#if defined(STAR_USE_SDL)
        return std::make_unique<sdl::SDLFileDialog>();
#else
    #error "No platform file dialog implementation available!"
#endif
    }
} // namespace star::platform
