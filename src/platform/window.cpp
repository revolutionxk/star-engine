#include "star/platform/window.hpp"

#include "star/core/common.hpp"
#include "star/platform/sdl/sdl_window.hpp"

namespace star::platform {
    std::unique_ptr<Window> Window::create_platform_window() {
#if defined(STAR_USE_SDL)
        return std::make_unique<sdl::SDLWindow>();
#else
    #error "No platform window implementation available!"
#endif
    }
} // namespace star::platform
