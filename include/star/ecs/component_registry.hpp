#pragma once

#include <functional>
#include <string_view>
#include <vector>

#include <flecs.h>

#include <nlohmann/json.hpp>

#include "star/core/meta/visitors/json_serializer.hpp"
#include "star/core/types.hpp"

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

    class ComponentRegistry {
      public:
        struct Entry {
            const char* type_name{};
            RegistrationFlags flags{RegistrationFlags::None};

            std::function<bool(flecs::entity)> has;
            std::function<void(flecs::entity)> add;
            std::function<void(flecs::entity)> remove;
            std::function<nlohmann::json(flecs::entity)> serialize;
            std::function<void(flecs::entity, const nlohmann::json&)> deserialize;
        };

        template<meta::Reflected T>
        void register_component(RegistrationFlags flags = RegistrationFlags::None) {
            if constexpr (requires { requires meta::TypeInfo<T>::required; })
                if (flags == RegistrationFlags::None)
                    flags = RegistrationFlags::Required;

            Entry e;
            e.type_name = meta::type_name<T>().data();
            e.flags = flags;

            e.has = [](const flecs::entity ent) { return ent.has<T>(); };
            e.add = [](const flecs::entity ent) { ent.add<T>(); };
            e.remove = [](const flecs::entity ent) { ent.remove<T>(); };

            // TODO: I need to abstract this away from JSON at some point, but for now it's fine
            e.serialize = [](flecs::entity ent) -> nlohmann::json {
                if (const auto* comp = ent.try_get<T>())
                    return meta::to_json(*comp);
                return nullptr;
            };
            e.deserialize = [](flecs::entity ent, const nlohmann::json& j) {
                if (!ent.has<T>())
                    ent.add<T>();
                auto& comp = *ent.try_get_mut<T>();
                meta::from_json(j, comp);
            };

            m_entries.push_back(std::move(e));
        }

        template<meta::Reflected... Ts>
        void register_all() {
            (register_component<Ts>(), ...);
        }

        [[nodiscard]] const Entry* find(const std::string_view name) const noexcept {
            for (const auto& e : m_entries)
                if (e.type_name == name)
                    return &e;
            return nullptr;
        }

        [[nodiscard]] nlohmann::json serialize_entity(const flecs::entity entity) const {
            nlohmann::json j;
            for (const auto& e : m_entries) {
                if (!e.has(entity))
                    continue;
                auto comp_json = e.serialize(entity);
                if (!comp_json.is_null())
                    j[e.type_name] = std::move(comp_json);
            }
            return j;
        }

        void deserialize_entity(const flecs::entity entity, const nlohmann::json& j) const {
            for (const auto& e : m_entries) {
                if (!j.contains(e.type_name))
                    continue;
                if (!e.has(entity))
                    e.add(entity);
                e.deserialize(entity, j.at(e.type_name));
            }
        }

        [[nodiscard]] const std::vector<Entry>& entries() const noexcept {
            return m_entries;
        }

      private:
        std::vector<Entry> m_entries;
    };

} // namespace star::ecs
