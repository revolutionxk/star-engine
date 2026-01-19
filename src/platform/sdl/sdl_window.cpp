#include "star/platform/sdl/sdl_window.hpp"

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

    bool SDLWindow::create(const WindowConfig& config) {
        auto flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;

        if (config.resizable) {
            flags |= SDL_WINDOW_RESIZABLE;
        }

        m_window = SDL_CreateWindow(config.title.c_str(), config.width, config.height, flags);

        if (!m_window) {
            STAR_LOG_ERROR(LogCategory::Platform, "Failed to create SDL window: {}", SDL_GetError());
            return false;
        }

        STAR_LOG_INFO(LogCategory::Platform, "SDL window created: {} ({}x{})", config.title, config.width,
                      config.height);

        return Super::create(config);
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
                default:
                    break;
            }
        }
    }

    void* SDLWindow::handle() const {
        return m_window;
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
    }
} // namespace star::platform::sdl
