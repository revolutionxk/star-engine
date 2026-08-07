#pragma once

#include <any>
#include <string>
#include <string_view>
#include <typeindex>
#include <utility>
#include <vector>

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
            return m_entity.is_alive() && component_info(m_type) != nullptr && m_before.has_value() &&
                   m_after.has_value();
        }

      private:
        flecs::entity m_entity;
        std::type_index m_type;
        std::any m_before;
        std::any m_after;
        std::string m_label;
    };

    class AddComponentCommand final : public ICommand {
      public:
        AddComponentCommand(const flecs::entity entity, const std::type_index type, std::string label)
            : m_entity(entity), m_type(type), m_label(std::move(label)) {}

        void execute() override {
            if (const auto* info = component_info(m_type))
                info->add(m_entity);
        }

        void undo() override {
            if (const auto* info = component_info(m_type))
                info->remove(m_entity);
        }

        [[nodiscard]] std::string_view label() const override {
            return m_label;
        }

        [[nodiscard]] bool is_valid() const override {
            return m_entity.is_alive() && component_info(m_type) != nullptr;
        }

      private:
        flecs::entity m_entity;
        std::type_index m_type;
        std::string m_label;
    };

    class RemoveComponentCommand final : public ICommand {
      public:
        RemoveComponentCommand(const flecs::entity entity, const std::type_index type, std::string label)
            : m_entity(entity), m_type(type), m_saved(clone_component(entity, type)), m_label(std::move(label)) {}

        void execute() override {
            if (const auto* info = component_info(m_type))
                info->remove(m_entity);
        }

        void undo() override {
            restore_component(m_entity, m_type, m_saved);
        }

        [[nodiscard]] std::string_view label() const override {
            return m_label;
        }

        [[nodiscard]] bool is_valid() const override {
            return m_entity.is_alive() && component_info(m_type) != nullptr && m_saved.has_value();
        }

      private:
        flecs::entity m_entity;
        std::type_index m_type;
        std::any m_saved;
        std::string m_label;
    };

    struct SavedComponent {
        std::type_index type;
        std::any value;
    };

    inline std::vector<SavedComponent> clone_all_components(const flecs::entity entity) {
        std::vector<SavedComponent> saved;
        for (const auto& type_info : reflection::TypeRegistry::instance().all_types()) {
            std::any boxed = clone_component(entity, type_info.type);
            if (boxed.has_value())
                saved.push_back({type_info.type, std::move(boxed)});
        }
        return saved;
    }

    class DestroyEntityCommand final : public ICommand {
      public:
        DestroyEntityCommand(const flecs::entity entity, std::string label)
            : m_world(entity.world()), m_id(entity.id()), m_name(entity.name().c_str()),
              m_parent(entity.parent().id()), m_saved(clone_all_components(entity)), m_label(std::move(label)) {}

        void execute() override {
            m_world.entity(m_id).destruct();
        }

        void undo() override {
            auto entity = m_world.make_alive(m_id);
            if (!m_name.empty())
                entity.set_name(m_name.c_str());
            if (m_parent != 0)
                entity.child_of(m_world.entity(m_parent));
            for (const auto& [type, value] : m_saved)
                restore_component(entity, type, value);
        }

        [[nodiscard]] std::string_view label() const override {
            return m_label;
        }

        [[nodiscard]] bool is_valid() const override {
            return m_id != 0;
        }

      private:
        flecs::world m_world;
        flecs::entity_t m_id;
        std::string m_name;
        flecs::entity_t m_parent;
        std::vector<SavedComponent> m_saved;
        std::string m_label;
    };

    class CreateEntityCommand final : public ICommand {
      public:
        CreateEntityCommand(const flecs::world world, std::string name, const flecs::entity parent, std::string label)
            : m_world(world), m_name(std::move(name)), m_parent(parent.id()), m_label(std::move(label)) {}

        void execute() override {
            auto entity = m_id == 0 ? (m_name.empty() ? m_world.entity() : m_world.entity(m_name.c_str()))
                                    : m_world.make_alive(m_id);
            if (m_id != 0 && !m_name.empty())
                entity.set_name(m_name.c_str());
            if (m_parent != 0)
                entity.child_of(m_world.entity(m_parent));
            m_id = entity.id();
        }

        void undo() override {
            m_world.entity(m_id).destruct();
        }

        [[nodiscard]] flecs::entity_t created_id() const noexcept {
            return m_id;
        }

        [[nodiscard]] std::string_view label() const override {
            return m_label;
        }

        [[nodiscard]] bool is_valid() const override {
            return true;
        }

      private:
        flecs::world m_world;
        std::string m_name;
        flecs::entity_t m_parent;
        flecs::entity_t m_id{0};
        std::string m_label;
    };
} // namespace star::editor
