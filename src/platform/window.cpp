#include "star/platform/window.hpp"

#include "star/core/common.hpp"
#include "star/platform/sdl/sdl_window.hpp"

namespace star::platform {
    bool Window::create(const VideoMode& video_mode) {
        m_video_mode = video_mode;
        m_opened = true;
        return true;
    }

    std::unique_ptr<Window> Window::create_platform_window() {
#if defined(STAR_USE_SDL)
        return std::make_unique<sdl::SDLWindow>();
#else
    #error "No platform window implementation available!"
#endif
    }
} // namespace star::platform
