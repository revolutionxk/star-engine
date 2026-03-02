#pragma once

#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "attributes.hpp"

namespace star::meta {
    template<typename Owner, typename Value, typename... Attrs>
    struct FieldInfo {
        using owner_type = Owner;
        using value_type = Value;
        using attr_tuple = std::tuple<Attrs...>;

        std::string_view name;
        Value Owner::* ptr;
        attr_tuple attributes{};

        template<typename A>
        [[nodiscard]] static constexpr bool has_attr() noexcept {
            return (std::is_same_v<A, Attrs> || ...);
        }

        [[nodiscard]] static constexpr bool has_enum_options() noexcept {
            return (attr::is_enum_options_v<Attrs> || ...);
        }

        template<typename A>
        [[nodiscard]] constexpr std::optional<A> get_attr() const noexcept {
            std::optional<A> result;
            std::apply(
                [&result]<typename... As>(const As&... a) {
                    (
                        [&result, &a]() noexcept {
                            if constexpr (std::is_same_v<std::decay_t<decltype(a)>, A>) {
                                if (!result.has_value())
                                    result = a;
                            }
                        }(),
                        ...);
                },
                attributes);
            return result;
        }

        template<typename A>
        [[nodiscard]] constexpr A get_attr_or(A default_val) const noexcept {
            return get_attr<A>().value_or(default_val);
        }

        template<typename Fn>
        constexpr void visit_enum_options(Fn&& fn) const noexcept {
            std::apply(
                [&fn]<typename... As>(const As&... a) {
                    (
                        [&fn, &a]() noexcept {
                            if constexpr (attr::is_enum_options_v<std::decay_t<decltype(a)>>) {
                                fn(a);
                            }
                        }(),
                        ...);
                },
                attributes);
        }

        template<attr::Attribute A>
        [[nodiscard]] constexpr auto operator|(this const FieldInfo& self, A&& a) noexcept {
            auto new_attrs = std::tuple_cat(self.attributes, std::make_tuple(std::forward<A>(a)));
            return FieldInfo<Owner, Value, Attrs..., std::decay_t<A>>{self.name, self.ptr, std::move(new_attrs)};
        }
    };

    template<typename Owner, typename Value>
    [[nodiscard]] constexpr auto field(std::string_view name, Value Owner::* ptr) noexcept {
        return FieldInfo<Owner, Value>{name, ptr};
    }

} // namespace star::meta
