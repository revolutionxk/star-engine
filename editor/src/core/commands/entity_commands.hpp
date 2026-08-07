#pragma once

#include <any>
#include <string>
#include <string_view>
#include <typeindex>

#include <flecs.h>

#include "star/core/reflection/type_registry.hpp"
#include "star/ecs/component_registry.hpp"

#include "command.hpp"

namespace star::editor {
    inline const reflection::RuntimeTypeInfo* runtime_type(const std::type_index type) {
        const auto result = reflection::TypeRegistry::instance().find(type);
        return result.has_value() ? *result : nullptr;
    }

    inline const ecs::EcsComponentInfo* component_info(const std::type_index type) {
        const auto* info = runtime_type(type);
        if (!info)
            return nullptr;
        return reflection::TypeRegistry::instance().get_extension<ecs::EcsComponentInfo>(type);
    }

    inline std::any clone_component(const flecs::entity entity, const std::type_index type) {
        const auto* info = runtime_type(type);
        const auto* ecs_info = component_info(type);
        if (!info || !ecs_info || !info->clone || !ecs_info->has(entity))
            return {};

        void* ptr = ecs_info->get_mut_ptr(entity);
        if (!ptr)
            return {};

        return info->clone(ptr);
    }

    inline bool restore_component(const flecs::entity entity, const std::type_index type, const std::any& boxed) {
        const auto* info = runtime_type(type);
        const auto* ecs_info = component_info(type);
        if (!info || !ecs_info || !info->restore || !boxed.has_value())
            return false;

        if (!ecs_info->has(entity))
            ecs_info->add(entity);

        void* ptr = ecs_info->get_mut_ptr(entity);
        if (!ptr)
            return false;

        return info->restore(ptr, boxed);
    }

    class SetComponentCommand final : public ICommand {
      public:
        SetComponentCommand(const flecs::entity entity, const std::type_index type, std::any before, std::any after,
                            std::string label)
            : m_entity(entity), m_type(type), m_before(std::move(before)), m_after(std::move(after)),
              m_label(std::move(label)) {}

        void execute() override {
            restore_component(m_entity, m_type, m_after);
        }

        void undo() override {
            restore_component(m_entity, m_type, m_before);
        }

        [[nodiscard]] std::string_view label() const override {
            return m_label;
        }

        [[nodiscard]] bool is_valid() const override {
            return m_entity.is_alive() && runtime_type(m_type) != nullptr && m_before.has_value() &&
                   m_after.has_value();
        }

      private:
        flecs::entity m_entity;
        std::type_index m_type;
        std::any m_before;
        std::any m_after;
        std::string m_label;
    };
} // namespace star::editor
