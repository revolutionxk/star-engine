#include "star/core/uuid.hpp"

#include <array>
#include <charconv>
#include <random>

namespace star::detail {
    u64 splitmix64(u64 x) noexcept {
        x += 0x9e3779b97f4a7c15ull;
        x = (x ^ x >> 30) * 0xbf58476d1ce4e5b9ull;
        x = (x ^ x >> 27) * 0x94d049bb133111ebull;
        return x ^ x >> 31;
    }

    std::mt19937_64& engine() {
        static thread_local std::mt19937_64 generator{std::random_device{}()};
        return generator;
    }

    bool read_hex(const std::string_view text, u64& out) {
        u64 value = 0;
        const auto* first = text.data();
        const auto* last = first + text.size();
        const auto [ptr, ec] = std::from_chars(first, last, value, 16);
        if (ec != std::errc{} || ptr != last) {
            return false;
        }
        out = value;
        return true;
    }

    void write_hex(u64 value, char* out) {
        constexpr char digits[] = "0123456789abcdef";
        for (int i = 15; i >= 0; --i) {
            out[i] = digits[value & 0xfull];
            value >>= 4;
        }
    }
} // namespace star::detail

namespace star {
    Uuid Uuid::generate() {
        std::uniform_int_distribution<u64> distribution;

        u64 high = distribution(detail::engine());
        u64 low = distribution(detail::engine());

        if (high == 0 && low == 0) {
            high = 1;
        }
        return Uuid{high, low};
    }

    Uuid Uuid::derive(const Uuid& parent, const std::string_view tag) {
        u64 high = detail::splitmix64(parent.high() ^ 0x2545f4914f6cdd1dull);
        u64 low = detail::splitmix64(parent.low());

        for (const char c : tag) {
            high = detail::splitmix64(high ^ static_cast<u64>(static_cast<unsigned char>(c)));
            low = detail::splitmix64(low + high);
        }

        if (high == 0 && low == 0) {
            high = 1;
        }
        return Uuid{high, low};
    }

    std::optional<Uuid> Uuid::parse(const std::string_view text) {
        if (text.size() != 32) {
            return std::nullopt;
        }

        u64 high = 0;
        u64 low = 0;
        if (!detail::read_hex(text.substr(0, 16), high) || !detail::read_hex(text.substr(16, 16), low)) {
            return std::nullopt;
        }
        return Uuid{high, low};
    }

    std::string Uuid::to_string() const {
        std::array<char, 32> buffer{};
        detail::write_hex(m_high, buffer.data());
        detail::write_hex(m_low, buffer.data() + 16);
        return std::string{buffer.data(), buffer.size()};
    }
} // namespace star
