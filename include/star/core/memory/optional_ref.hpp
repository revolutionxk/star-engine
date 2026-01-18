#pragma once

#include <type_traits>

namespace star {
    template<typename T>
    class OptionalRef final {
        T* _value;

      public:
        using std_t = std::optional<std::reference_wrapper<T>>;

        OptionalRef() noexcept : _value(nullptr) {}

        explicit OptionalRef(std::nullopt_t) noexcept : _value(nullptr) {}

        explicit OptionalRef(const std_t& value) noexcept : _value(value ? &value.value().get() : nullptr) {}

        OptionalRef(const OptionalRef& other) noexcept = default;

        OptionalRef(OptionalRef&& other) noexcept : _value(other._value) {
            other.reset();
        }

        OptionalRef(T* value) noexcept : _value(value) {}

        OptionalRef(T& value) noexcept : _value(&value) {}

        OptionalRef& operator=(std::nullopt_t) noexcept {
            reset();
            return *this;
        }

        OptionalRef& operator=(const OptionalRef& other) noexcept = default;

        void reset() noexcept {
            _value = nullptr;
        }

        [[nodiscard]] T* operator->() const {
            if (_value == nullptr) {
                throw std::bad_optional_access();
            }
            return _value;
        }

        [[nodiscard]] T& operator*() const {
            if (_value == nullptr) {
                throw std::bad_optional_access();
            }
            return *_value;
        }

        explicit operator bool() const noexcept {
            return _value != nullptr;
        }

        [[nodiscard]] bool empty() const noexcept {
            return _value == nullptr;
        }

        [[nodiscard]] T& value() const {
            if (!*this) {
                throw std::bad_optional_access();
            }
            return *_value;
        }

        [[nodiscard]] T* ptr() const noexcept {
            return _value;
        }

        explicit operator std_t() const noexcept {
            if (_value == nullptr) {
                return std::nullopt;
            }
            return *_value;
        }

        explicit operator OptionalRef<const T>() const noexcept {
            return _value;
        }

        [[nodiscard]] bool operator==(const OptionalRef& other) const noexcept {
            return _value == other._value;
        }

        [[nodiscard]] bool operator!=(const OptionalRef& other) const noexcept {
            return !operator==(other);
        }
    };
} // namespace star
