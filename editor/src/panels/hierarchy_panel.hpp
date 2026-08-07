#pragma once

#include <array>

#include "../core/commands/command_stack.hpp"
#include "../core/editor_events.hpp"
#include "editor_window.hpp"
#include "panel.hpp"
#include "star/scene/scene.hpp"
#include "star/scene/scene_manager.hpp"

namespace star::editor {
    class HierarchyPanel final : public Panel {
      public:
        explicit HierarchyPanel(EditorWindow* editor_window) : Panel("Hierarchy"), m_editor_window(editor_window) {}

        void on_imgui_render() override;

        void set_command_stack(CommandStack* stack) noexcept {
            m_command_stack = stack;
        }

      private:
        void render_search_bar();
        void render_entity_list();
        void render_entity_node(flecs::entity entity);
        void render_context_menu();
        void filter_entities();
        void create_empty_entity();
        void create_camera_entity();
        void import_gltf_model() const;
        static void on_entity_selected(flecs::entity& entity);

        std::array<char, 256> m_search_buffer{};
        flecs::entity m_selected_entity;
        EditorWindow* m_editor_window = nullptr;
        CommandStack* m_command_stack{nullptr};
    };
} // namespace star::editor
