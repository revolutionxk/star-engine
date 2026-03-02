#pragma once

#include <string_view>
#include <typeindex>

#include "field_info.hpp"
#include "star/platform/input/input.hpp"

namespace star::reflection {
    template<typename T>
    struct TypeInfo;

    template<typename T>
    concept Reflected = requires {
        { TypeInfo<T>::name } -> std::convertible_to<std::string_view>;
        { TypeInfo<T>::fields };
    };

    template<Reflected T>
    [[nodiscard]] constexpr std::string_view type_name() noexcept {
        return TypeInfo<T>::name;
    }

    template<Reflected T>
    [[nodiscard]] constexpr const auto& type_fields() noexcept {
        return TypeInfo<T>::fields;
    }

    template<Reflected T>
    [[nodiscard]] constexpr std::string_view script_name() noexcept {
        if constexpr (requires { TypeInfo<T>::script_name; })
            return TypeInfo<T>::script_name;
        else
            return TypeInfo<T>::name;
    }

    template<Reflected T>
    [[nodiscard]] constexpr std::string_view type_category() noexcept {
        if constexpr (requires { TypeInfo<T>::category; })
            return TypeInfo<T>::category;
        else
            return {};
    }

    template<Reflected T>
    inline constexpr std::size_t field_count_v = std::tuple_size_v<std::decay_t<decltype(TypeInfo<T>::fields)>>;

    template<typename T>
    concept EcsComponent = Reflected<T> && requires { requires TypeInfo<T>::is_component; };
} // namespace star::reflection
