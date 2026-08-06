#pragma once

#include <array>

#include "star/core/types.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/math/math.hpp"
#include "star/rendering/render_target.hpp"
#include "star/rendering/renderer.hpp"

namespace star::graphics {
    class Device;
    struct Shader;
} // namespace star::graphics

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::systems {
    class RenderSystem;
} // namespace star::systems

namespace star::rendering {
    inline constexpr u32 MAX_SHADOW_CASCADES = 4;

    class ShadowPass final : public IRenderPass {
      public:
        ShadowPass(graphics::Device& device, resources::ResourceManager& resources,
                   systems::RenderSystem& render_system);
        ~ShadowPass() override;

        [[nodiscard]] std::string get_name() const override {
            return "ShadowPass";
        }

        [[nodiscard]] u8 get_priority() const override {
            return 50;
        }

        void render(const RenderContext& ctx) override;

        struct Settings {
            bool enabled = true;
            f32 depth_bias = 0.05f;
            f32 normal_bias = 1.5f;
            f32 max_distance = 120.0f;
            f32 split_lambda = 0.85f;
            u32 cascade_count = 3;
            bool visualize_cascades = false;
        };

        [[nodiscard]] Settings& settings() noexcept {
            return m_settings;
        }

      private:
        static constexpr u32 CASCADE_TILE_SIZE = 2048;
        static constexpr u32 ATLAS_SIZE = CASCADE_TILE_SIZE * 2;

        struct Cascade {
            Matrix4 view_proj{Matrix4::identity()};
            f32 split_far{0.0f};
            f32 texel_world_size{0.0f};
            f32 depth_range{1.0f};
        };

        static bool find_sun_direction(const RenderScene& scene, Vector3& out_travel_dir);

        [[nodiscard]] u32 active_cascade_count() const;

        void compute_splits(f32 near_plane, f32 far_plane, std::array<f32, MAX_SHADOW_CASCADES + 1>& out) const;

        [[nodiscard]] Cascade fit_cascade(const Viewport& viewport, const Vector3& light_dir, f32 split_near,
                                          f32 split_far) const;

        static void tile_offset(u32 index, u32& out_x, u32& out_y);

        graphics::Device& m_device;
        resources::ResourceManager& m_resources;
        systems::RenderSystem& m_render_system;

        graphics::ResourceHandle<graphics::Shader> m_shadow_shader;
        RenderTarget m_target;
        Settings m_settings;
        u64 m_last_render_frame = ~0ull;
    };
} // namespace star::rendering
