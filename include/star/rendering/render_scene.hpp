#pragma once

#include <optional>
#include <string>
#include <vector>

#include "star/core/types.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/math/math.hpp"
#include "star/rendering/material_property.hpp"

namespace star::resources {
    struct Mesh;
    struct Material;
} // namespace star::resources

namespace star::rendering {

    struct CameraSnapshot {
        f32 fov_y{45.0f};
        f32 near_plane{0.1f};
        f32 far_plane{1000.0f};
        f32 aspect_ratio{16.0f / 9.0f};
        Vector3 position{};
        Quaternion rotation{0.f, 0.f, 0.f, 1.f};
        Vector3 scale{1.f, 1.f, 1.f};
    };

    struct LightSnapshot {
        enum class Type : u8 {
            Directional,
            Point,
            Spot
        };

        Type type{Type::Directional};
        Vector3 position{0.f, 0.f, 0.f};
        Vector3 direction{0.f, -1.f, 0.f};
        Color4 color{1.f, 1.f, 1.f, 1.f};
        f32 intensity{1.0f};
        f32 ambient_intensity{0.0f};
        f32 range{10.0f};
        f32 inner_cone_angle_deg{30.0f};
        f32 outer_cone_angle_deg{45.0f};
        bool cast_shadows{false};
    };

    struct AtmosphereSnapshot {
        bool enabled{false};
        f32 turbidity{2.5f};
        f32 sun_elevation{0.4f};
        f32 sun_azimuth{0.0f};
        f32 sun_intensity{1.0f};
    };

    struct RenderableSnapshot {
        Matrix4 model_matrix{Matrix4::identity()};
        Vector3 world_position{};
        graphics::ResourceHandle<resources::Mesh> mesh{};
        graphics::ResourceHandle<resources::Material> material{};
        std::vector<MaterialProperty> parameters{};
        u64 entity_id{0};
        u32 layer{0};
        bool is_transparent{false};
        bool has_bounds{false};
        AABB bounds{};
    };

    struct RenderScene {
        std::optional<CameraSnapshot> primary_camera;
        std::vector<LightSnapshot> lights;
        std::optional<AtmosphereSnapshot> atmosphere;
        std::vector<RenderableSnapshot> renderables;
        std::string name;
        bool active{false};

        void clear() noexcept {
            primary_camera.reset();
            lights.clear();
            atmosphere.reset();
            renderables.clear();
            active = false;
            name.clear();
        }
    };
} // namespace star::rendering
