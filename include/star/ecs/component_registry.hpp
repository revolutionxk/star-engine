#pragma once

#include <functional>

#include <flecs.h>

#include <nlohmann/json.hpp>

#include "star/core/reflection/reflect.hpp"
#include "star/core/types.hpp"
#include "star/ecs/serialization.hpp"

namespace star::ecs {
    enum class RegistrationFlags : u32 {
        None = 0,
        Required = 1 << 0,
        Hidden = 1 << 1,
    };

    constexpr RegistrationFlags operator|(RegistrationFlags a, RegistrationFlags b) {
        return static_cast<RegistrationFlags>(static_cast<u32>(a) | static_cast<u32>(b));
    }

    constexpr bool has_flag(RegistrationFlags flags, RegistrationFlags flag) {
        return (static_cast<u32>(flags) & static_cast<u32>(flag)) != 0;
    }

    struct EcsComponentInfo {
        RegistrationFlags flags{RegistrationFlags::None};
        bool is_empty{false};

        std::function<bool(flecs::entity)> has;
        std::function<void(flecs::entity)> add;
        std::function<void(flecs::entity)> remove;
        std::function<void*(flecs::entity)> get_mut_ptr;
        std::function<nlohmann::json(flecs::entity)> serialize;
        std::function<void(flecs::entity, const nlohmann::json&)> deserialize;
        std::function<void(flecs::world&, std::string_view)> register_world;
    };

    template<reflection::EcsComponent T>
    void register_component(RegistrationFlags flags = RegistrationFlags::None) {
        if constexpr (requires { requires reflection::TypeInfo<T>::required; })
            if (flags == RegistrationFlags::None)
                flags = RegistrationFlags::Required;

        reflection::TypeRegistry::instance().register_type<T>();

        EcsComponentInfo info;
        info.flags = flags;
        info.is_empty = std::is_empty_v<T>;
        info.has = [](flecs::entity e) { return e.has<T>(); };
        info.add = [](flecs::entity e) { e.add<T>(); };
        info.remove = [](flecs::entity e) { e.remove<T>(); };
        info.get_mut_ptr = [](flecs::entity e) -> void* {
            if constexpr (std::is_empty_v<T>)
                return nullptr;
            else
                return e.try_get_mut<T>();
        };
        info.serialize = [](flecs::entity e) -> nlohmann::json {
            if constexpr (std::is_empty_v<T>) {
                return nlohmann::json::object();
            } else {
                if (const auto* c = e.try_get<T>())
                    return to_json(*c);
                return nullptr;
            }
        };
        info.deserialize = [](flecs::entity e, const nlohmann::json& j) {
            if (!e.has<T>())
                e.add<T>();
            if constexpr (!std::is_empty_v<T>)
                from_json(j, *e.try_get_mut<T>());
        };
        info.register_world = [](flecs::world& w, std::string_view name) { w.component<T>(name.data()); };

        reflection::TypeRegistry::instance().extend<EcsComponentInfo>(typeid(T), std::move(info));
    }

    inline nlohmann::json serialize_entity(flecs::entity entity) {
        nlohmann::json j;
        for (const auto& type_info : reflection::TypeRegistry::instance().all_types()) {
            const auto* ecs = reflection::TypeRegistry::instance().get_extension<EcsComponentInfo>(type_info.type);
            if (!ecs || !ecs->has(entity))
                continue;
            auto comp_json = ecs->serialize(entity);
            if (!comp_json.is_null())
                j[std::string{type_info.name}] = std::move(comp_json);
        }
        return j;
    }

    inline void deserialize_entity(flecs::entity entity, const nlohmann::json& j) {
        for (const auto& type_info : reflection::TypeRegistry::instance().all_types()) {
            const auto* ecs = reflection::TypeRegistry::instance().get_extension<EcsComponentInfo>(type_info.type);
            if (!ecs || !j.contains(std::string{type_info.name}))
                continue;
            if (!ecs->has(entity))
                ecs->add(entity);
            ecs->deserialize(entity, j.at(std::string{type_info.name}));
        }
    }

    inline void register_world_components(flecs::world& world) {
        for (const auto& type_info : reflection::TypeRegistry::instance().all_types()) {
            const auto* ecs = reflection::TypeRegistry::instance().get_extension<EcsComponentInfo>(type_info.type);
            if (!ecs)
                continue;
            ecs->register_world(world, type_info.name);
        }
    }
} // namespace star::ecs

#define STAR_REGISTER_COMPONENT(T, ...)                                                                                \
    namespace star::reflection::detail {                                                                               \
        [[maybe_unused]] static const bool STAR_META_CONCAT(_comp_, __COUNTER__) = [] {                                \
            ::star::ecs::register_component<T>(__VA_ARGS__);                                                           \
            return true;                                                                                               \
        }();                                                                                                           \
    }
