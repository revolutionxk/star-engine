#include "star/platform/input/input.hpp"

#include "platform/sdl/sdl_input.hpp"

namespace star::platform {
    std::unique_ptr<Input> Input::create() {
        return std::make_unique<sdl::SDLInput>();
    }
} // namespace star::platform
