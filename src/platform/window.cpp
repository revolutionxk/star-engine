#include "star/platform/window.hpp"

#include "sdl/sdl_window.hpp"
#include "star/core/common.hpp"

namespace star::platform {
    bool Window::create(const WindowConfiguration& config) {
        m_configuration = config;
        m_opened = true;
        return true;
    }

    std::unique_ptr<Window> Window::create() {
#if defined(STAR_USE_SDL)
        return std::make_unique<sdl::SDLWindow>();
#else
    #error "No platform window implementation available!"
#endif
    }
} // namespace star::platform
