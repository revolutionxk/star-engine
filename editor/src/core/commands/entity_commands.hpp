#pragma once

#include <any>
#include <cstdint>
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
    struct TagPresence {};

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

    inline std::string entity_name(const flecs::entity entity) {
        const char* name = entity.name().c_str();
        return name != nullptr ? std::string{name} : std::string{};
    }

    inline flecs::entity_t lookup_in_scope(const flecs::world& world, const flecs::entity_t parent, const char* name) {
        if (parent != 0 && world.entity(parent).is_alive())
            return world.entity(parent).lookup(name).id();
        return world.lookup(name).id();
    }

    inline bool name_taken(const flecs::world& world, const flecs::entity_t parent, const char* name,
                           const flecs::entity_t self = 0) {
        const flecs::entity_t found = lookup_in_scope(world, parent, name);
        return found != 0 && found != self;
    }

    inline std::string unique_child_name(const flecs::world& world, const flecs::entity_t parent,
                                         const std::string& base, const flecs::entity_t self = 0) {
        if (base.empty() || !name_taken(world, parent, base.c_str(), self))
            return base;

        for (int suffix = 1; suffix < 100000; ++suffix) {
            std::string candidate = base + " (" + std::to_string(suffix) + ")";
            if (!name_taken(world, parent, candidate.c_str(), self))
                return candidate;
        }
        return base;
    }

    inline bool index_is_revivable(const flecs::world& world, const flecs::entity_t id) {
        if (id == 0)
            return false;
        const flecs::entity_t current = ecs_get_alive(world.c_ptr(), static_cast<std::uint32_t>(id));
        return current == 0 || current == id;
    }

    inline std::any clone_component(const flecs::entity entity, const std::type_index type) {
        const auto* info = runtime_type(type);
        const auto* ecs_info = component_info(type);
        if (!info || !ecs_info || !ecs_info->has(entity))
            return {};

        if (ecs_info->is_empty)
            return std::any{TagPresence{}};

        if (!info->clone)
            return {};

        void* ptr = ecs_info->get_mut_ptr(entity);
        if (!ptr)
            return {};

        return info->clone(ptr);
    }

    inline bool restore_component(const flecs::entity entity, const std::type_index type, const std::any& boxed) {
        const auto* info = runtime_type(type);
        const auto* ecs_info = component_info(type);
        if (!info || !ecs_info || !boxed.has_value())
            return false;

        if (ecs_info->is_empty) {
            if (!ecs_info->has(entity))
                ecs_info->add(entity);
            return true;
        }

        if (!info->restore)
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

    struct SavedEntity {
        flecs::entity_t id{0};
        std::string name;
        flecs::entity_t parent{0};
        std::vector<SavedComponent> components;
    };

    inline void collect_subtree(const flecs::entity entity, std::vector<SavedEntity>& out) {
        out.push_back({entity.id(), entity_name(entity), entity.parent().id(), clone_all_components(entity)});

        std::vector<flecs::entity_t> child_ids;
        entity.children([&child_ids](const flecs::entity child) { child_ids.push_back(child.id()); });

        const auto world = entity.world();
        for (const flecs::entity_t child_id : child_ids)
            collect_subtree(world.entity(child_id), out);
    }

    class DestroyEntityCommand final : public ICommand {
      public:
        DestroyEntityCommand(const flecs::entity entity, std::string label)
            : m_world(entity.world()), m_label(std::move(label)) {
            collect_subtree(entity, m_saved);
        }

        void execute() override {
            if (m_saved.empty())
                return;

            const auto entity = m_world.entity(m_saved.front().id);
            if (entity.is_alive())
                entity.destruct();
        }

        void undo() override {
            for (const auto& saved : m_saved) {
                if (!index_is_revivable(m_world, saved.id))
                    continue;

                auto entity = m_world.make_alive(saved.id);
                if (saved.parent != 0 && m_world.entity(saved.parent).is_alive())
                    entity.child_of(m_world.entity(saved.parent));
                if (!saved.name.empty())
                    entity.set_name(unique_child_name(m_world, saved.parent, saved.name, saved.id).c_str());
                for (const auto& [type, value] : saved.components)
                    restore_component(entity, type, value);
            }
        }

        [[nodiscard]] std::string_view label() const override {
            return m_label;
        }

        [[nodiscard]] bool is_valid() const override {
            if (m_saved.empty())
                return false;

            for (const auto& saved : m_saved)
                if (!index_is_revivable(m_world, saved.id))
                    return false;

            const auto& root = m_saved.front();
            return root.parent == 0 || m_world.entity(root.parent).is_alive();
        }

      private:
        flecs::world m_world;
        std::vector<SavedEntity> m_saved;
        std::string m_label;
    };

    class CreateEntityCommand final : public ICommand {
      public:
        CreateEntityCommand(const flecs::world world, std::string name, const flecs::entity parent, std::string label)
            : m_world(world), m_base_name(std::move(name)), m_parent(parent.id()), m_label(std::move(label)) {}

        void execute() override {
            auto entity = index_is_revivable(m_world, m_id) ? m_world.make_alive(m_id) : m_world.entity();

            if (m_parent != 0 && m_world.entity(m_parent).is_alive())
                entity.child_of(m_world.entity(m_parent));

            if (!m_base_name.empty()) {
                if (m_name.empty() || name_taken(m_world, m_parent, m_name.c_str(), entity.id()))
                    m_name = unique_child_name(m_world, m_parent, m_base_name, entity.id());
                entity.set_name(m_name.c_str());
            }

            m_id = entity.id();
        }

        void undo() override {
            if (m_id == 0)
                return;

            const auto entity = m_world.entity(m_id);
            if (entity.is_alive())
                entity.destruct();
        }

        [[nodiscard]] flecs::entity_t created_id() const noexcept {
            return m_id;
        }

        [[nodiscard]] std::string_view label() const override {
            return m_label;
        }

        [[nodiscard]] bool is_valid() const override {
            return (m_id == 0 || index_is_revivable(m_world, m_id)) &&
                   (m_parent == 0 || m_world.entity(m_parent).is_alive());
        }

      private:
        flecs::world m_world;
        std::string m_base_name;
        std::string m_name;
        flecs::entity_t m_parent;
        flecs::entity_t m_id{0};
        std::string m_label;
    };

    class ReparentCommand final : public ICommand {
      public:
        ReparentCommand(const flecs::entity entity, const flecs::entity new_parent, std::string label)
            : m_entity(entity), m_old_parent(entity.parent().id()), m_new_parent(new_parent.id()),
              m_label(std::move(label)) {}

        void execute() override {
            apply(m_new_parent);
        }

        void undo() override {
            apply(m_old_parent);
        }

        [[nodiscard]] std::string_view label() const override {
            return m_label;
        }

        [[nodiscard]] bool is_valid() const override {
            return m_entity.is_alive();
        }

      private:
        void apply(const flecs::entity_t parent) const {
            if (parent == 0)
                m_entity.mut(m_entity.world()).remove(flecs::ChildOf, flecs::Wildcard);
            else
                m_entity.mut(m_entity.world()).child_of(m_entity.world().entity(parent));
        }

        flecs::entity m_entity;
        flecs::entity_t m_old_parent;
        flecs::entity_t m_new_parent;
        std::string m_label;
    };

    class RenameEntityCommand final : public ICommand {
      public:
        RenameEntityCommand(const flecs::entity entity, std::string new_name, std::string label)
            : m_entity(entity), m_old_name(entity_name(entity)), m_new_name(std::move(new_name)),
              m_label(std::move(label)) {}

        void execute() override {
            apply(m_new_name);
        }

        void undo() override {
            apply(m_old_name);
        }

        [[nodiscard]] std::string_view label() const override {
            return m_label;
        }

        [[nodiscard]] bool is_valid() const override {
            return m_entity.is_alive();
        }

      private:
        void apply(const std::string& name) const {
            const auto world = m_entity.world();
            auto target = m_entity.mut(world);
            if (name.empty()) {
                target.set_name(nullptr);
                return;
            }
            target.set_name(unique_child_name(world, target.parent().id(), name, target.id()).c_str());
        }

        flecs::entity m_entity;
        std::string m_old_name;
        std::string m_new_name;
        std::string m_label;
    };
} // namespace star::editor
