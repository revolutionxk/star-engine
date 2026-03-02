#pragma once

#include <array>
#include <string_view>
#include <type_traits>

#include "star/core/types.hpp"

namespace star::meta::attr {
    struct Speed {
        f32 value{0.1f};
    };

    struct Range {
        f32 min{0.f}, max{0.f};
    };

    struct Color {};

    struct Normalized {};

    struct Multiline {};

    struct Tooltip {
        std::string_view text;
    };

    struct Category {
        std::string_view name;
    };

    struct ReadOnly {};

    struct HideInEditor {};

    struct Indent {
        u32 level{1};
    };

    template<size_t N>
    struct EnumOptions {
        std::array<std::string_view, N> labels{};

        template<typename... Ts>
            requires(sizeof...(Ts) == N) && (std::is_constructible_v<std::string_view, Ts> && ...)
        constexpr explicit EnumOptions(Ts&&... ls) noexcept : labels{std::string_view(ls)...} {}
    };

    template<typename... Ts>
    EnumOptions(Ts...) -> EnumOptions<sizeof...(Ts)>;

    struct Transient {};

    struct SerialName {
        std::string_view name;
    };

    struct ScriptName {
        std::string_view name;
    };

    struct NoScript {};

    struct ScriptReadOnly {};

    template<typename A>
    concept Attribute = std::is_trivially_copyable_v<A>;

    template<typename T>
    struct is_enum_options : std::false_type {};

    template<size_t N>
    struct is_enum_options<EnumOptions<N>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_enum_options_v = is_enum_options<T>::value;
} // namespace star::meta::attr
