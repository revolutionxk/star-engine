#include "star/platform/sdl/sdl_window.hpp"

namespace star::platform::sdl {
    bool SDLWindow::create(const WindowConfig& config) {
        auto flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;

        if (config.resizable) {
            flags |= SDL_WINDOW_RESIZABLE;
        }

        m_window = SDL_CreateWindow(config.title, config.width, config.height, flags);

        if (!m_window) {
            STAR_CORE_ERROR("Failed to create SDL window: {}", SDL_GetError());
            return false;
        }

        STAR_CORE_INFO("SDL window created: {} ({}x{})", config.title, config.width, config.height);

        return true;
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
        return Vector2(static_cast<f32>(width), static_cast<f32>(height));
    }
} // namespace star::platform::sdl
