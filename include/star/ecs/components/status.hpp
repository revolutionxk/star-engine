#pragma once

namespace star::components {
    struct Active {
        bool value{true};

        explicit operator bool() const {
            return value;
        }
    };

    struct Disabled {};

    struct Ready {};
} // namespace star::components
