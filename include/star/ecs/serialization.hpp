#pragma once

#include <expected>
#include <filesystem>
#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

#include "star/core/reflection/reflect.hpp"
#include "star/math/math.hpp"
#include "star/math/quaternion.hpp"
#include "star/math/vector2.hpp"
#include "star/math/vector3.hpp"
#include "star/math/vector4.hpp"

namespace star::math {

    template<std::floating_point T>
    inline void to_json(nlohmann::json& j, const Vector2T<T>& v) {
        j = {{"x", v.x}, {"y", v.y}};
    }

    template<std::floating_point T>
    inline void from_json(const nlohmann::json& j, Vector2T<T>& v) {
        if (j.contains("x"))
            j.at("x").get_to(v.x);
        if (j.contains("y"))
            j.at("y").get_to(v.y);
    }

    template<std::floating_point T>
    inline void to_json(nlohmann::json& j, const Vector3T<T>& v) {
        j = {{"x", v.x}, {"y", v.y}, {"z", v.z}};
    }

    template<std::floating_point T>
    inline void from_json(const nlohmann::json& j, Vector3T<T>& v) {
        if (j.contains("x"))
            j.at("x").get_to(v.x);
        if (j.contains("y"))
            j.at("y").get_to(v.y);
        if (j.contains("z"))
            j.at("z").get_to(v.z);
    }

    template<std::floating_point T>
    inline void to_json(nlohmann::json& j, const Vector4T<T>& v) {
        j = {{"x", v.x}, {"y", v.y}, {"z", v.z}, {"w", v.w}};
    }

    template<std::floating_point T>
    inline void from_json(const nlohmann::json& j, Vector4T<T>& v) {
        if (j.contains("x"))
            j.at("x").get_to(v.x);
        if (j.contains("y"))
            j.at("y").get_to(v.y);
        if (j.contains("z"))
            j.at("z").get_to(v.z);
        if (j.contains("w"))
            j.at("w").get_to(v.w);
    }

    template<std::floating_point T>
    inline void to_json(nlohmann::json& j, const QuaternionT<T>& q) {
        j = {{"x", q.x}, {"y", q.y}, {"z", q.z}, {"w", q.w}};
    }

    template<std::floating_point T>
    inline void from_json(const nlohmann::json& j, QuaternionT<T>& q) {
        if (j.contains("x"))
            j.at("x").get_to(q.x);
        if (j.contains("y"))
            j.at("y").get_to(q.y);
        if (j.contains("z"))
            j.at("z").get_to(q.z);
        if (j.contains("w"))
            j.at("w").get_to(q.w);
    }

} // namespace star::math

namespace star::ecs {

    namespace detail {
        template<typename FD>
        [[nodiscard]] constexpr std::string_view serial_key(const FD& d) noexcept {
            if constexpr (FD::template has_attr<reflection::attr::SerialName>()) {
                if (auto sn = d.template get_attr<reflection::attr::SerialName>())
                    return sn->name;
            }
            return d.name;
        }
    } // namespace detail

    class JsonWriteVisitor {
      public:
        explicit JsonWriteVisitor(nlohmann::json& out) : m_out{out} {}

        template<typename FD, typename Value>
        void operator()(const FD& d, const Value& v) const {
            if constexpr (FD::template has_attr<reflection::attr::Transient>())
                return;

            const auto key = std::string{detail::serial_key(d)};
            if constexpr (std::is_enum_v<Value>)
                m_out[key] = static_cast<int>(v);
            else
                m_out[key] = v;
        }

      private:
        nlohmann::json& m_out;
    };

    class JsonReadVisitor {
      public:
        explicit JsonReadVisitor(const nlohmann::json& in) : m_in{in} {}

        template<typename FD, typename Value>
        void operator()(const FD& d, Value& v) const {
            if constexpr (FD::template has_attr<reflection::attr::Transient>())
                return;

            const auto key = std::string{detail::serial_key(d)};
            if (!m_in.contains(key))
                return;

            try {
                if constexpr (std::is_enum_v<Value>)
                    v = static_cast<Value>(m_in.at(key).get<int>());
                else
                    m_in.at(key).get_to(v);
            } catch (...) {
            }
        }

      private:
        const nlohmann::json& m_in;
    };

    template<reflection::Reflected T>
    [[nodiscard]] nlohmann::json to_json(const T& component) {
        nlohmann::json j;
        reflection::for_each_field(component, JsonWriteVisitor{j});
        return j;
    }

    template<reflection::Reflected T>
    void from_json(const nlohmann::json& j, T& component) {
        reflection::for_each_field(component, JsonReadVisitor{j});
    }

    enum class SerializeError {
        IoError,
        ParseError
    };

    template<reflection::Reflected T>
    [[nodiscard]] std::expected<nlohmann::json, SerializeError> to_json_safe(const T& component) noexcept {
        try {
            return to_json(component);
        } catch (...) {
            return std::unexpected(SerializeError::IoError);
        }
    }

    template<reflection::Reflected T>
    [[nodiscard]] std::expected<void, SerializeError> save_to_file(const T& component,
                                                                   const std::filesystem::path& path) noexcept {
        try {
            std::ofstream f(path);
            if (!f)
                return std::unexpected(SerializeError::IoError);
            f << to_json(component).dump(4);
            return {};
        } catch (...) {
            return std::unexpected(SerializeError::IoError);
        }
    }

    template<reflection::Reflected T>
    [[nodiscard]] std::expected<void, SerializeError> load_from_file(T& component,
                                                                     const std::filesystem::path& path) noexcept {
        try {
            std::ifstream f(path);
            if (!f)
                return std::unexpected(SerializeError::IoError);
            nlohmann::json j;
            f >> j;
            from_json(j, component);
            return {};
        } catch (...) {
            return std::unexpected(SerializeError::ParseError);
        }
    }

} // namespace star::ecs
