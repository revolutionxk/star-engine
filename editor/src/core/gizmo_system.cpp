#include "gizmo_system.hpp"

#include "star/math/math.hpp"

namespace star::editor {
    bool GizmoSystem::draw_and_process(const ImVec2& image_pos, const ImVec2& image_size,
                                       const rendering::Viewport& vp) const {
        if (!m_entity || !m_entity->is_valid())
            return false;

        const auto* transform = m_entity->try_get_mut<components::Transform>();
        if (!transform || image_size.x <= 0.0f || image_size.y <= 0.0f)
            return false;

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        ImGuizmo::SetRect(image_pos.x, image_pos.y, image_size.x, image_size.y);

        Matrix4 model = transform->to_matrix();
        if (ImGuizmo::Manipulate(vp.view_matrix().data(), vp.projection_matrix().data(), to_imguizmo_op(m_operation),
                                 to_imguizmo_mode(m_space), model.m)) {
            decompose(model, *transform);
        }

        return ImGuizmo::IsUsing();
    }

    ImGuizmo::OPERATION GizmoSystem::to_imguizmo_op(const Operation op) {
        switch (op) {
            case Operation::Rotate:
                return ImGuizmo::ROTATE;
            case Operation::Scale:
                return ImGuizmo::SCALE;
            default:
                return ImGuizmo::TRANSLATE;
        }
    }

    ImGuizmo::MODE GizmoSystem::to_imguizmo_mode(const Space s) {
        return s == Space::World ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
    }

    void GizmoSystem::decompose(const Matrix4& mat, const components::Transform& t) {
        t.position = {mat.m[12], mat.m[13], mat.m[14]};

        const Vector3 col0{mat.m[0], mat.m[1], mat.m[2]};
        const Vector3 col1{mat.m[4], mat.m[5], mat.m[6]};
        const Vector3 col2{mat.m[8], mat.m[9], mat.m[10]};
        t.scale = {col0.length(), col1.length(), col2.length()};

        Matrix4 rot = Matrix4::identity();
        if (t.scale.x > 1e-6f) {
            rot.m[0] = col0.x / t.scale.x;
            rot.m[1] = col0.y / t.scale.x;
            rot.m[2] = col0.z / t.scale.x;
        }
        if (t.scale.y > 1e-6f) {
            rot.m[4] = col1.x / t.scale.y;
            rot.m[5] = col1.y / t.scale.y;
            rot.m[6] = col1.z / t.scale.y;
        }
        if (t.scale.z > 1e-6f) {
            rot.m[8] = col2.x / t.scale.z;
            rot.m[9] = col2.y / t.scale.z;
            rot.m[10] = col2.z / t.scale.z;
        }
        t.rotation = rot.to_quaternion();
    }
} // namespace star::editor
