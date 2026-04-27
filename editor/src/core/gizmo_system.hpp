#pragma once

#include <optional>

// clang-format off
#include <imgui.h>
#include <ImGuizmo.h>
#include <flecs.h>
// clang-format on

#include "star/ecs/components/transform.hpp"
#include "star/rendering/viewport.hpp"

namespace star::editor {
    class GizmoSystem {
      public:
        enum class Operation {
            Translate,
            Rotate,
            Scale
        };
        enum class Space {
            World,
            Local
        };

        void set_entity(const std::optional<flecs::entity>& e) noexcept {
            m_entity = e;
        }

        void set_operation(const Operation op) noexcept {
            m_operation = op;
        }

        void set_space(const Space s) noexcept {
            m_space = s;
        }

        [[nodiscard]] Operation operation() const noexcept {
            return m_operation;
        }

        [[nodiscard]] Space space() const noexcept {
            return m_space;
        }

        [[nodiscard]] static bool is_using() noexcept {
            return ImGuizmo::IsUsing();
        }

        [[nodiscard]] static bool is_over() noexcept {
            return ImGuizmo::IsOver();
        }

        bool draw_and_process(const ImVec2& image_pos, const ImVec2& image_size, const rendering::Viewport& vp) const;

      private:
        static ImGuizmo::OPERATION to_imguizmo_op(Operation op);
        static ImGuizmo::MODE to_imguizmo_mode(Space s);
        static void decompose(const Matrix4& mat, const components::Transform& t);

        std::optional<flecs::entity> m_entity;
        Operation m_operation = Operation::Translate;
        Space m_space = Space::World;
    };
} // namespace star::editor
