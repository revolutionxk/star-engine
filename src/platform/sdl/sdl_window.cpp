#include "sdl_window.hpp"

#include "backends/imgui_impl_sdl3.h"
#include "sdl_input.hpp"

#ifdef STAR_PLATFORM_WINDOWS
#  include "star/core/platform_win32.hpp"
#endif

namespace star::platform::sdl {
    int SDLWindow::s_ref_count = 0;

    SDLWindow::SDLWindow() {
        if (s_ref_count++ == 0) {
            if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
                STAR_LOG_ERROR(LogCategory::Platform, "Failed to initialize SDL: {}", SDL_GetError());
            }
        }
    }

    SDLWindow::~SDLWindow() {
        destroy();
        if (--s_ref_count == 0) {
            SDL_Quit();
        }
    }

    bool SDLWindow::create(const WindowConfiguration& config) {
        auto flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;

        if (config.video_mode.resizable) {
            flags |= SDL_WINDOW_RESIZABLE;
        }

        switch (config.video_mode.mode) {
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

        m_window = SDL_CreateWindow(config.title.c_str(), config.video_mode.size.x, config.video_mode.size.y, flags);

        if (!m_window) {
            STAR_LOG_ERROR(LogCategory::Platform, "Failed to create SDL window: {}", SDL_GetError());
            return false;
        }

#ifdef STAR_PLATFORM_MACOS
        m_metal_view = SDL_Metal_CreateView(m_window);
        if (!m_metal_view) {
            STAR_LOG_ERROR(LogCategory::Platform, "Failed to create SDL Metal view: {}", SDL_GetError());
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
            return false;
        }
#endif

        STAR_LOG_INFO(LogCategory::Platform, "SDL window created: {}x{}", config.video_mode.size.x,
                      config.video_mode.size.y);

        return Super::create(config);
    }

    void SDLWindow::destroy() {
        if (!m_window) {
            return;
        }

#ifdef STAR_PLATFORM_MACOS
        if (m_metal_view) {
            SDL_Metal_DestroyView(m_metal_view);
            m_metal_view = nullptr;
        }
#endif

        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    void SDLWindow::set_resize_callback(ResizeCallback callback) {
        m_resize_callback = std::move(callback);
    }

    void SDLWindow::poll_events() {
        if (!m_window) {
            return;
        }

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            const bool in_relative_mode = SDL_GetWindowRelativeMouseMode(m_window);
            const bool is_release = event.type == SDL_EVENT_MOUSE_BUTTON_UP || event.type == SDL_EVENT_KEY_UP;
            if (!in_relative_mode || is_release)
                ImGui_ImplSDL3_ProcessEvent(&event); // TODO: temp fix, move to ImGui backend later with proper checks

            if (m_input_manager) {
                m_input_manager->process_event(event);
            }

            switch (event.type) {
                case SDL_EVENT_QUIT:
                    m_opened = false;
                    break;

                case SDL_EVENT_WINDOW_RESIZED:
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
                    const auto new_size = size();
                    const auto width = static_cast<u32>(new_size.x);
                    const auto height = static_cast<u32>(new_size.y);
                    STAR_LOG_DEBUG(LogCategory::Platform, "Window resized to {}x{}", width, height);

                    if (m_resize_callback) {
                        m_resize_callback(width, height);
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
        return m_window;
    }

    void* SDLWindow::native_handle() const {
        if (!m_window) {
            return nullptr;
        }

#ifdef STAR_PLATFORM_WINDOWS
        const auto hwnd = static_cast<HWND>(
            SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

        return hwnd;
#elifdef STAR_PLATFORM_LINUX
        if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
            const auto x11_window =
                SDL_GetNumberProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
            return reinterpret_cast<void*>(static_cast<uintptr_t>(x11_window));
        }
        if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
            const auto surface = static_cast<struct wl_surface*>(SDL_GetPointerProperty(
                SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr));
            return surface;
        }
#elifdef STAR_PLATFORM_MACOS
        return m_metal_view ? SDL_Metal_GetLayer(m_metal_view) : nullptr;
#endif
        return nullptr;
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
        return {native_handle(), display_handle(), native_handle_type()};
    }

    NativeWindowHandleType SDLWindow::native_handle_type() const {
#ifdef STAR_PLATFORM_WINDOWS
        return NativeWindowHandleType::Win32;
#elif defined(STAR_PLATFORM_LINUX)
        if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
            return NativeWindowHandleType::X11;
        }
        if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
            return NativeWindowHandleType::Wayland;
        }
#elif defined(STAR_PLATFORM_MACOS)
        return NativeWindowHandleType::Cocoa;
#endif
        return NativeWindowHandleType::Default;
    }

    Vector2 SDLWindow::size() const {
        if (!m_window) {
            return Vector2::zero();
        }

        i32 width, height;
        SDL_GetWindowSizeInPixels(m_window, &width, &height);
        return {static_cast<f32>(width), static_cast<f32>(height)};
    }

    f32 SDLWindow::pixel_density() const {
        if (!m_window) {
            return 1.0f;
        }

        const f32 density = SDL_GetWindowPixelDensity(m_window);
        return density > 0.0f ? density : 1.0f;
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
    }

    void SDLWindow::set_input_manager(Input* input_manager) {
        m_input_manager = dynamic_cast<SDLInput*>(input_manager);
    }

    void SDLWindow::set_relative_mouse_mode(const bool enabled) {
        SDL_SetWindowRelativeMouseMode(m_window, enabled);
    }
} // namespace star::platform::sdl
