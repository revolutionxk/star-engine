#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

#include "../type_info.hpp"

namespace star::reflection {

    template<Reflected T, typename V>
    constexpr void for_each_field(T& obj, V&& visitor) {
        std::apply([&obj, &visitor](const auto&... info) { (visitor(info, obj.*info.ptr), ...); }, TypeInfo<T>::fields);
    }

    template<Reflected T, typename V>
    constexpr void for_each_field(const T& obj, V&& visitor) {
        std::apply([&obj, &visitor](const auto&... info) { (visitor(info, obj.*info.ptr), ...); }, TypeInfo<T>::fields);
    }

    template<Reflected T, typename V>
    constexpr void for_each_field_desc(V&& visitor) {
        std::apply([&visitor](const auto&... info) { (visitor(info), ...); }, TypeInfo<T>::fields);
    }

    template<Reflected T, typename V>
    constexpr bool apply_to_field(T& obj, std::string_view name, V&& visitor) {
        return []<std::size_t... Is>(T& o, std::string_view n, V&& vis, std::index_sequence<Is...>) -> bool {
            return ([&]() -> bool {
                if (constexpr auto& info = std::get<Is>(TypeInfo<T>::fields); info.name == n) {
                    vis(info, o.*info.ptr);
                    return true;
                }
                return false;
            }() || ...);
        }(obj, name, std::forward<V>(visitor), std::make_index_sequence<field_count_v<T>>{});
    }

    template<Reflected T, typename A>
    consteval std::size_t count_fields_with_attr() noexcept {
        std::size_t n = 0;
        std::apply([&n]<typename... Ts>(
                       const Ts&... info) { ((n += std::decay_t<Ts>::template has_attr<A>() ? 1u : 0u), ...); },
                   TypeInfo<T>::fields);
        return n;
    }

    template<Reflected T, typename A>
    inline constexpr bool any_field_has_attr_v = count_fields_with_attr<T, A>() > 0;

} // namespace star::reflection
