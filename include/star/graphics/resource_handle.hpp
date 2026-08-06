#pragma once
#include <cstddef>
#include <functional>
#include <limits>

#include "star/core/types.hpp"

namespace star::graphics {

    template<typename>
    struct ResourceHandle {
        static constexpr u32 INVALID_ID = std::numeric_limits<u32>::max();

        u32 id{INVALID_ID};
        u32 generation{0};

        [[nodiscard]] bool is_valid() const noexcept {
            return id != INVALID_ID;
        }

        void reset() noexcept {
            id = INVALID_ID;
            generation = 0;
        }

        bool operator==(const ResourceHandle&) const noexcept = default;
        bool operator!=(const ResourceHandle&) const noexcept = default;
    };

} // namespace star::graphics

template<typename T>
struct std::hash<star::graphics::ResourceHandle<T>> {
    std::size_t operator()(const star::graphics::ResourceHandle<T>& h) const noexcept {
        const star::u64 packed = static_cast<star::u64>(h.id) << 32 | h.generation;
        return std::hash<star::u64>{}(packed);
    }
};
