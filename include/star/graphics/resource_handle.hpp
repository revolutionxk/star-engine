#pragma once

namespace star::graphics {
    template<typename>
    struct ResourceHandle {
        u32 id{0};
        u32 generation{0};

        bool is_valid() const {
            return id != 0;
        }
    };
} // namespace star::graphics
