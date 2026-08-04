#include <memory>
#include <string>

#include "star/application/app_window.hpp"
#include "star/application/application.hpp"
#include "star/ecs/components/camera.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/rendering/components/atmosphere.hpp"
#include "star/rendering/components/light.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/rendering/renderer.hpp"
#include "star/rendering/viewport.hpp"
#include "star/resources/material/material.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/scene/entity.hpp"
#include "star/scene/scene.hpp"
#include "star/scene/scene_manager.hpp"

using namespace star;

namespace {
    class BasicWindow final : public application::AppWindow {
      public:
        BasicWindow() : AppWindow("Star Engine - Basic Sample") {}

      protected:
        bool on_initialize() override {
            m_viewport = std::make_unique<rendering::Viewport>(device());
            renderer().set_active_viewport(m_viewport.get());

            auto* scene = scene_manager().create_scene("BasicSample");
            if (!scene) {
                STAR_LOG_ERROR(LogCategory::Application, "Failed to create sample scene");
                return false;
            }

            scene_manager().set_active_scene(scene->name());
            build_scene(*scene);
            return true;
        }

        void on_update(const f32 delta_time) override {
            const auto size = window().size();
            m_viewport->resize(static_cast<u32>(size.x), static_cast<u32>(size.y));

            m_spin += delta_time * 0.9f;

            if (auto* transform = m_cube.get_mut<components::Transform>()) {
                transform->rotation = Quaternion::from_euler(0.0f, m_spin, 0.0f);
            }
        }

      private:
        graphics::ResourceHandle<resources::Material> make_material(const std::string& name, const Vector4& albedo,
                                                                    const f32 metallic, const f32 roughness) const {
            auto material = std::make_unique<resources::Material>();
            material->shader = resources().default_shader();
            material->albedo_color = albedo;
            material->metallic = metallic;
            material->roughness = roughness;
            return resources().create_material(name, std::move(material));
        }

        void build_scene(const scene::Scene& scene) {
            scene.create_entity("Main Camera")
                .set<components::Transform>({.position = Vector3{0.0f, 2.0f, 6.0f}})
                .set<components::Camera>({})
                .add<components::PrimaryCamera>();

            scene.create_entity("Sky").set<components::Atmosphere>({
                .turbidity = 2.5f,
                .sun_elevation = 0.6f,
                .sun_azimuth = 0.4f,
                .sun_intensity = 1.6f,
            });

            scene.create_entity("Sun").set<components::Light>({
                .type = components::Light::Type::Directional,
                .direction = Vector3{-0.4f, -0.8f, -0.45f}.normalized(),
                .color = Color4{1.0f, 0.97f, 0.92f, 1.0f},
                .intensity = 1.0f,
                .cast_shadows = true,
            });

            const auto ground_material = make_material("Ground", Vector4{0.32f, 0.33f, 0.36f, 1.0f}, 0.0f, 0.9f);
            scene.create_entity("Ground")
                .set<components::Transform>({
                    .position = Vector3{0.0f, -1.0f, 0.0f},
                    .scale = Vector3{20.0f, 1.0f, 20.0f},
                })
                .set<components::MeshRenderer>({
                    .mesh = resources().plane_mesh(),
                    .material = ground_material,
                });

            const auto cube_material = make_material("Cube", Vector4{0.2f, 0.5f, 1.0f, 1.0f}, 0.0f, 0.35f);
            m_cube = scene.create_entity("Cube")
                         .set<components::Transform>({})
                         .set<components::MeshRenderer>({
                             .mesh = resources().cube_mesh(),
                             .material = cube_material,
                         });
        }

        std::unique_ptr<rendering::Viewport> m_viewport;
        scene::Entity m_cube;
        f32 m_spin = 0.0f;
    };

    class BasicApp final : public application::Application {
      public:
        explicit BasicApp(const application::CommandLineArgs& args) : Application(args) {}

      protected:
        bool on_initialize() override {
            if (!create_window<BasicWindow>()) {
                STAR_LOG_ERROR(LogCategory::Application, "Failed to create sample window");
                return false;
            }
            return true;
        }
    };
} // namespace

STAR_RUN_APPLICATION(BasicApp);
