#include "inspector_panel.hpp"
#include "editor_context.hpp"
#include "editor_app.hpp"
#include "star/scene/scene.hpp"
#include "star/scene/transform.hpp"
#include "star/scene/camera.hpp"
#include "star/render/renderer_components.hpp"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace star::editor {
    InspectorPanel::InspectorPanel(EditorContext &context)
        : EditorPanel("Inspector", context) {}

    void InspectorPanel::on_imgui_render() {
        ImGui::Begin("Inspector");

        const auto selected = get_context().get_selected_entity();
        if (!selected.has_value()) {
            ImGui::Text("No entity selected");
            ImGui::End();
            return;
        }

        auto *scene = get_context().get_app()->get_active_scene();
        if (!scene || !scene->is_valid_entity(selected.value())) {
            ImGui::Text("Invalid entity");
            ImGui::End();
            return;
        }

        Entity entity = selected.value();

        // Entity header
        ImGui::Text("Entity: %u", static_cast<uint32_t>(entity));
        ImGui::Separator();

        // Draw components
        draw_components();

        ImGui::Separator();

        // Add component button
        if (ImGui::Button("Add Component")) {
            ImGui::OpenPopup("AddComponent");
        }

        draw_add_component_menu();

        // Delete entity button
        ImGui::Separator();
        if (ImGui::Button("Delete Entity")) {
            scene->destroy_entity(entity);
            get_context().clear_selection();
        }

        ImGui::End();
    }

    void InspectorPanel::draw_components() {
        draw_transform_component();
        draw_camera_component();
        draw_light_component();
        draw_mesh_renderer_component();
        draw_material_component();
    }

    void InspectorPanel::draw_transform_component() {
        auto selected = get_context().get_selected_entity();
        if (!selected.has_value()) return;

        auto *scene = get_context().get_app()->get_active_scene();
        if (!scene) return;

        Entity entity = selected.value();
        auto *transform = scene->get_component<Transform>(entity);
        if (!transform) return;

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto pos = transform->get_position();
            if (ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.1f)) {
                transform->set_position(pos);
            }

            auto euler = transform->get_euler_angles();
            if (ImGui::DragFloat3("Rotation", glm::value_ptr(euler), 1.0f)) {
                transform->set_euler_angles(euler);
            }

            auto scale = transform->get_scale();
            if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.1f)) {
                transform->set_scale(scale);
            }
        }
    }

    void InspectorPanel::draw_camera_component() {
        auto selected = get_context().get_selected_entity();
        if (!selected.has_value()) return;

        auto *scene = get_context().get_app()->get_active_scene();
        if (!scene) return;

        Entity entity = selected.value();
        auto *camera = scene->get_component<Camera>(entity);
        if (!camera) return;

        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
            // float fov = camera->get_fov();
            // if (ImGui::SliderFloat("FOV", &fov, 1.0f, 120.0f)) {
            //     camera->set_fov(fov);
            // }
            //
            // float near_plane = camera->get_near_plane();
            // if (ImGui::DragFloat("Near Plane", &near_plane, 0.01f, 0.01f, 1000.0f)) {
            //     camera->set_near_plane(near_plane);
            // }
            //
            // float far_plane = camera->get_far_plane();
            // if (ImGui::DragFloat("Far Plane", &far_plane, 1.0f, 0.1f, 10000.0f)) {
            //     camera->set_far_plane(far_plane);
            // }
        }
    }

    void InspectorPanel::draw_light_component() {
        auto selected = get_context().get_selected_entity();
        if (!selected.has_value()) return;

        auto *scene = get_context().get_app()->get_active_scene();
        if (!scene) return;

        Entity entity = selected.value();
        auto *light = scene->get_component<Light>(entity);
        if (!light) return;

        if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            const char *types[] = {"Directional", "Point", "Spot"};
            int current_type = static_cast<int>(light->get_type());
            if (ImGui::Combo("Type", &current_type, types, IM_ARRAYSIZE(types))) {
                light->set_type(static_cast<LightType>(current_type));
            }

            auto color = light->get_color();
            if (ImGui::ColorEdit3("Color", glm::value_ptr(color))) {
                light->set_color(color);
            }

            float intensity = light->get_intensity();
            if (ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 100.0f)) {
                light->set_intensity(intensity);
            }
        }
    }

    void InspectorPanel::draw_mesh_renderer_component() {
        auto selected = get_context().get_selected_entity();
        if (!selected.has_value()) return;

        auto *scene = get_context().get_app()->get_active_scene();
        if (!scene) return;

        Entity entity = selected.value();
        auto *mesh_renderer = scene->get_component<MeshRenderer>(entity);
        if (!mesh_renderer) return;

        if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
            // ImGui::Text("Mesh: %s", mesh_renderer->has_mesh() ? "Yes" : "No");
            // ImGui::Text("Material: %s", mesh_renderer->has_material() ? "Yes" : "No");
        }
    }

    void InspectorPanel::draw_material_component() {
        auto selected = get_context().get_selected_entity();
        if (!selected.has_value()) return;

        auto *scene = get_context().get_app()->get_active_scene();
        if (!scene) return;

        Entity entity = selected.value();
        auto *material_component = scene->get_component<MaterialComponent>(entity);
        if (!material_component) return;

        if (ImGui::CollapsingHeader("Material Component", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto material = material_component->get_material();
            if (!material) {
                ImGui::Text("No material assigned.");
                return;
            }

            if (material->get_type() == MaterialType::Unlit) {
                if (auto *unlit = dynamic_cast<UnlitMaterial*>(material.get())) {
                    if (glm::vec4 color = unlit->get_color(); ImGui::ColorEdit4("Color", glm::value_ptr(color))) {
                        unlit->set_color(color);
                    }
                }
            }

            if (material->get_type() == MaterialType::Standard) {
                auto *standard = dynamic_cast<StandardMaterial*>(material.get());
                if (standard) {
                    glm::vec4 base_color = standard->get_base_color();
                    if (ImGui::ColorEdit4("Base Color", glm::value_ptr(base_color))) {
                        standard->set_base_color(base_color);
                    }
                    float metallic = standard->get_metallic();
                    if (ImGui::DragFloat("Metallic", &metallic, 0.01f, 0.0f, 1.0f)) {
                        standard->set_metallic(metallic);
                    }
                    float roughness = standard->get_roughness();
                    if (ImGui::DragFloat("Roughness", &roughness, 0.01f, 0.0f, 1.0f)) {
                        standard->set_roughness(roughness);
                    }
                    glm::vec3 emissive = standard->get_emissive();
                    if (ImGui::ColorEdit3("Emissive", glm::value_ptr(emissive))) {
                        standard->set_emissive(emissive);
                    }
                }
            }
        }
    }

    void InspectorPanel::draw_add_component_menu() {
        if (ImGui::BeginPopup("AddComponent")) {
            auto selected = get_context().get_selected_entity();
            if (!selected.has_value()) {
                ImGui::EndPopup();
                return;
            }

            auto *scene = get_context().get_app()->get_active_scene();
            if (!scene) {
                ImGui::EndPopup();
                return;
            }

            Entity entity = selected.value();

            if (!scene->has_component<Camera>(entity)) {
                if (ImGui::MenuItem("Camera")) {
                    auto &camera = scene->add_component<Camera>(entity);
                    camera.set_perspective(60.0f, 0.1f, 1000.0f);
                    ImGui::CloseCurrentPopup();
                }
            }

            if (!scene->has_component<Light>(entity)) {
                if (ImGui::MenuItem("Light")) {
                    auto &light = scene->add_component<Light>(entity);
                    light.set_type(LightType::Directional);
                    light.set_color(glm::vec3(1.0f));
                    light.set_intensity(1.0f);
                    ImGui::CloseCurrentPopup();
                }
            }

            if (!scene->has_component<MeshRenderer>(entity)) {
                if (ImGui::MenuItem("Mesh Renderer")) {
                    scene->add_component<MeshRenderer>(entity);
                    ImGui::CloseCurrentPopup();
                }
            }

            if (!scene->has_component<MaterialComponent>(entity)) {
                if (ImGui::MenuItem("Material")) {
                    scene->add_component<MaterialComponent>(entity);
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }
    }
}
