#pragma once

#include <any>
#include <expected>
#include <functional>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "star/core/export.hpp"
#include "visitors/visitor.hpp"

namespace star::reflection {
    struct AnyAttr {
        std::type_index type;
        std::any value;

        template<typename A>
        [[nodiscard]] static AnyAttr from(const A& a) {
            return {typeid(A), a};
        }

        template<typename A>
        [[nodiscard]] std::optional<A> as() const {
            if (type != typeid(A))
                return std::nullopt;
            return std::any_cast<A>(value);
        }

        template<typename A>
        [[nodiscard]] bool is() const noexcept {
            return type == typeid(A);
        }
    };

    struct AnyRef {
        void* data{};
        std::type_index type;
        bool is_const{false};

        template<typename T>
        [[nodiscard]] T* as() noexcept {
            if (is_const || type != typeid(T))
                return nullptr;
            return static_cast<T*>(data);
        }

        template<typename T>
        [[nodiscard]] const T* as() const noexcept {
            if (type != typeid(T))
                return nullptr;
            return static_cast<const T*>(data);
        }

        [[nodiscard]] bool is_valid() const noexcept {
            return data != nullptr;
        }
    };

    enum class RegistryError {
        TypeNotFound,
        FieldNotFound,
        TypeMismatch
    };

    struct RuntimeField {
        std::string_view name;
        std::string_view script_name;
        std::type_index value_type{typeid(void)};
        std::vector<AnyAttr> attributes;
        std::vector<std::string_view> enum_labels; // populated for enum fields with EnumOptions

        std::function<AnyRef(void*)> get_mut;
        std::function<AnyRef(const void*)> get;
        std::function<bool(void*, const AnyRef&)> set;

        template<typename A>
        [[nodiscard]] bool has_attr() const noexcept {
            return std::ranges::any_of(attributes, [](const AnyAttr& a) { return a.type == typeid(A); });
        }

        template<typename A>
        [[nodiscard]] std::optional<A> find_attr() const noexcept {
            auto it = std::ranges::find_if(attributes, [](const AnyAttr& a) { return a.type == typeid(A); });
            return it != attributes.end() ? it->template as<A>() : std::nullopt;
        }
    };

    struct RuntimeTypeInfo {
        std::string_view name;
        std::string_view script_name;
        std::string_view category;
        std::type_index type{typeid(void)};
        std::vector<RuntimeField> fields;
        std::unordered_map<std::type_index, std::any> extensions;

        std::function<std::any(const void*)> clone;
        std::function<bool(void*, const std::any&)> restore;

        [[nodiscard]] const RuntimeField* find_field(const std::string_view field_name) const noexcept {
            const auto it = std::ranges::find_if(fields, [&](const RuntimeField& f) { return f.name == field_name; });
            return it != fields.end() ? &*it : nullptr;
        }

        [[nodiscard]] const RuntimeField* find_field_by_script_name(const std::string_view sname) const noexcept {
            const auto it = std::ranges::find_if(fields, [&](const RuntimeField& f) { return f.script_name == sname; });
            return it != fields.end() ? &*it : nullptr;
        }
    };

    class STAR_EXPORT TypeRegistry {
      public:
        static TypeRegistry& instance() {
            static TypeRegistry s;
            return s;
        }

        TypeRegistry(const TypeRegistry&) = delete;
        TypeRegistry& operator=(const TypeRegistry&) = delete;

        template<Reflected T>
        void register_type() {
            const std::type_index id{typeid(T)};
            if (m_by_id.contains(id))
                return;

            RuntimeTypeInfo info{
                .name = type_name<T>(), .script_name = script_name<T>(), .category = type_category<T>(), .type = id};

            for_each_field_desc<T>([&info]<typename T0>(const T0& field_desc) {
                using FD = std::decay_t<T0>;
                using ValueT = FD::value_type;

                RuntimeField rf;
                rf.name = field_desc.name;

                if (auto sn = field_desc.template get_attr<attr::ScriptName>())
                    rf.script_name = sn->name;
                else
                    rf.script_name = field_desc.name;

                rf.value_type = typeid(ValueT);

                rf.get_mut = [ptr = field_desc.ptr](void* obj) -> AnyRef {
                    auto& val = static_cast<T*>(obj)->*ptr;
                    return {&val, typeid(ValueT), false};
                };
                rf.get = [ptr = field_desc.ptr](const void* obj) -> AnyRef {
                    const auto& val = static_cast<const T*>(obj)->*ptr;
                    return {const_cast<ValueT*>(&val), typeid(ValueT), true};
                };
                rf.set = [ptr = field_desc.ptr](void* obj, const AnyRef& ref) -> bool {
                    if (ref.type != typeid(ValueT))
                        return false;
                    static_cast<T*>(obj)->*ptr = *static_cast<const ValueT*>(ref.data);
                    return true;
                };
                std::apply([&rf](const auto&... a) { (rf.attributes.emplace_back(AnyAttr::from(a)), ...); },
                           field_desc.attributes);

                field_desc.visit_enum_options([&rf]<std::size_t N>(const attr::EnumOptions<N>& opts) {
                    rf.enum_labels.reserve(N);
                    for (std::size_t i = 0; i < N; ++i)
                        rf.enum_labels.push_back(opts.labels[i]);
                });

                info.fields.emplace_back(std::move(rf));
            });

            info.clone = [](const void* obj) -> std::any {
                return std::any{*static_cast<const T*>(obj)};
            };
            info.restore = [](void* obj, const std::any& boxed) -> bool {
                const T* source = std::any_cast<T>(&boxed);
                if (!source)
                    return false;
                *static_cast<T*>(obj) = *source;
                return true;
            };

            const std::string name_key{info.name};
            RuntimeTypeInfo& stored = (m_by_id[id] = std::move(info));
            m_by_name[name_key] = &stored;
        }

        using Result = std::expected<const RuntimeTypeInfo*, RegistryError>;

        [[nodiscard]] Result find(const std::string_view name) const noexcept {
            const auto it = m_by_name.find(std::string{name});
            if (it == m_by_name.end())
                return std::unexpected(RegistryError::TypeNotFound);
            return it->second;
        }

        [[nodiscard]] Result find(std::type_index id) const noexcept {
            const auto it = m_by_id.find(id);
            if (it == m_by_id.end())
                return std::unexpected(RegistryError::TypeNotFound);
            return &it->second;
        }

        template<Reflected T>
        [[nodiscard]] Result find() const noexcept {
            return find(typeid(T));
        }

        template<typename T>
        std::expected<void, RegistryError> set_field(T& instance, const std::string_view field_name,
                                                     const AnyRef value) const {
            auto type_result = find(typeid(T));
            if (!type_result)
                return std::unexpected(type_result.error());

            const RuntimeField* rf = (*type_result)->find_field(field_name);
            if (!rf)
                return std::unexpected(RegistryError::FieldNotFound);

            if (!rf->set(&instance, value))
                return std::unexpected(RegistryError::TypeMismatch);

            return {};
        }

        [[nodiscard]] auto all_types() const noexcept {
            return m_by_id | std::views::values;
        }

        [[nodiscard]] std::size_t registered_count() const noexcept {
            return m_by_id.size();
        }

        template<typename Ext>
        void extend(const std::type_index id, Ext ext) {
            auto it = m_by_id.find(id);
            if (it == m_by_id.end())
                return;
            it->second.extensions[typeid(Ext)] = std::move(ext);
        }

        template<typename Ext>
        [[nodiscard]] const Ext* get_extension(std::type_index id) const noexcept {
            auto it = m_by_id.find(id);
            if (it == m_by_id.end())
                return nullptr;
            auto ext_it = it->second.extensions.find(typeid(Ext));
            if (ext_it == it->second.extensions.end())
                return nullptr;
            return std::any_cast<Ext>(&ext_it->second);
        }

        template<typename Ext, Reflected T>
        [[nodiscard]] const Ext* get_extension() const noexcept {
            return get_extension<Ext>(typeid(T));
        }

      private:
        TypeRegistry() = default;
        
        std::unordered_map<std::type_index, RuntimeTypeInfo> m_by_id;
        std::unordered_map<std::string, const RuntimeTypeInfo*> m_by_name;
    };
} // namespace star::reflection

#define STAR_META_CONCAT_IMPL(a, b) a##b
#define STAR_META_CONCAT(a, b) STAR_META_CONCAT_IMPL(a, b)
#define STAR_REGISTER_TYPE(T)                                                                                          \
    namespace star::reflection::detail {                                                                               \
        [[maybe_unused]] static const bool STAR_META_CONCAT(_reg_, __COUNTER__) = [] {                                 \
            ::star::reflection::TypeRegistry::instance().register_type<T>();                                           \
            return true;                                                                                               \
        }();                                                                                                           \
    }
