#include "sdl_window.hpp"

namespace star::platform::sdl {
    SDLWindow::SDLWindow() {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
            STAR_LOG_ERROR(LogCategory::Platform, "Failed to initialize SDL: {}", SDL_GetError());
        }
    }

    SDLWindow::~SDLWindow() {
        destroy();
        SDL_Quit();
    }

    bool SDLWindow::create(const VideoMode& video_mode) {
        auto flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;

        if (video_mode.resizable) {
            flags |= SDL_WINDOW_RESIZABLE;
        }

        switch (video_mode.mode) {
            case WindowMode::Fullscreen:
                flags |= SDL_WINDOW_FULLSCREEN;
                break;
            case WindowMode::Borderless:
                flags |= SDL_WINDOW_BORDERLESS;
                break;
            case WindowMode::Windowed:
            default:
                break;
        }

        m_window = SDL_CreateWindow("", video_mode.size.x, video_mode.size.y, flags);

        if (!m_window) {
            STAR_LOG_ERROR(LogCategory::Platform, "Failed to create SDL window: {}", SDL_GetError());
            return false;
        }

        STAR_LOG_INFO(LogCategory::Platform, "SDL window created: {}x{}", video_mode.size.x, video_mode.size.y);

        return Super::create(video_mode);
    }

    void SDLWindow::destroy() {
        if (!m_window) {
            return;
        }

        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    void SDLWindow::pool_events() {
        if (!m_window) {
            return;
        }

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    m_opened = false;
                    break;

                case SDL_EVENT_WINDOW_RESIZED:
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
                    const auto new_size = size();
                    STAR_LOG_DEBUG(LogCategory::Platform, "Window resized to {}x{}", static_cast<u32>(new_size.x),
                                   static_cast<u32>(new_size.y));

                    if (m_device_context) {
                        m_device_context->resize(static_cast<u32>(new_size.x), static_cast<u32>(new_size.y));
                    }
                    break;
                }

                case SDL_EVENT_WINDOW_MINIMIZED:
                    STAR_LOG_DEBUG(LogCategory::Platform, "Window minimized");
                    break;

                case SDL_EVENT_WINDOW_MAXIMIZED:
                    STAR_LOG_DEBUG(LogCategory::Platform, "Window maximized");
                    break;

                case SDL_EVENT_WINDOW_FOCUS_GAINED:
                    STAR_LOG_TRACE(LogCategory::Platform, "Window gained focus");
                    break;

                case SDL_EVENT_WINDOW_FOCUS_LOST:
                    STAR_LOG_TRACE(LogCategory::Platform, "Window lost focus");
                    break;

                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    STAR_LOG_INFO(LogCategory::Platform, "Window close requested");
                    m_opened = false;
                    break;

                default:
                    break;
            }
        }
    }

    void* SDLWindow::handle() const {
        if (!m_window) {
            return nullptr;
        }
        const auto hwnd = static_cast<HWND>(
            SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

        return hwnd;
    }

    void* SDLWindow::display_handle() const {
        if (!m_window) {
            return nullptr;
        }

#ifdef STAR_PLATFORM_LINUX
        if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
            const auto xdisplay =
                SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
            return xdisplay;
        }

        if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
            const auto display = static_cast<struct wl_display*>(SDL_GetPointerProperty(
                SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr));
            return display;
        }
#endif

        return nullptr;
    }

    PlatformData SDLWindow::platform_data() {
        return {handle(), display_handle()};
    }

    Vector2 SDLWindow::size() const {
        if (!m_window) {
            return Vector2::zero();
        }

        i32 width, height;
        SDL_GetWindowSize(m_window, &width, &height);
        return {static_cast<f32>(width), static_cast<f32>(height)};
    }

    void SDLWindow::set_title(const std::string& title) const {
        if (!m_window) {
            return;
        }

        SDL_SetWindowTitle(m_window, title.c_str());
    }

    void SDLWindow::set_size(const int width, const int height) {
        if (!m_window) {
            return;
        }

        SDL_SetWindowSize(m_window, width, height);

        if (m_device_context) {
            m_device_context->resize(static_cast<u32>(width), static_cast<u32>(height));
        }
    }
} // namespace star::platform::sdl
