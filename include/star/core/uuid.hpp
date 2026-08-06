#pragma once

#include <compare>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

#include "star/core/types.hpp"

namespace star {
    class UUID {
      public:
        constexpr UUID() = default;

        constexpr UUID(const u64 high, const u64 low) noexcept : m_high(high), m_low(low) {}

        [[nodiscard]] static UUID generate();
        [[nodiscard]] static UUID derive(const UUID& parent, std::string_view tag);
        [[nodiscard]] static std::optional<UUID> parse(std::string_view text);
        [[nodiscard]] std::string to_string() const;

        [[nodiscard]] constexpr bool is_valid() const noexcept {
            return m_high != 0 || m_low != 0;
        }

        [[nodiscard]] constexpr u64 high() const noexcept {
            return m_high;
        }

        [[nodiscard]] constexpr u64 low() const noexcept {
            return m_low;
        }

        constexpr bool operator==(const UUID&) const noexcept = default;
        constexpr auto operator<=>(const UUID&) const noexcept = default;

      private:
        u64 m_high{0};
        u64 m_low{0};
    };
} // namespace star

template<>
struct std::hash<star::UUID> {
    std::size_t operator()(const star::UUID& id) const noexcept {
        return std::hash<star::u64>{}(id.high()) ^ std::hash<star::u64>{}(id.low()) << 1;
    }
};
