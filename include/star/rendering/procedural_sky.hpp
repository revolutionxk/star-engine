#pragma once
#include "star/core/types.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/math/math.hpp"

namespace star::graphics {
    struct Shader;
    class DeviceContext;
} // namespace star::graphics

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::rendering {
    struct SkyParams {
        Vector3 sun_direction{0.0f, 1.0f, 0.0f};
        Vector3 sun_luminance{1.0f, 1.0f, 1.0f};
        Vector3 sky_luminance_xyz{0.972f, 1.0f, 1.79f};
        float perez_coeff[5][4]{};
        float exposition{0.02f};
        float sun_size{0.02f};
        float sun_bloom{3.0f};
        float time{0.0f};
    };

    class ProceduralSky {
      public:
        ProceduralSky() = default;
        ~ProceduralSky();

        ProceduralSky(const ProceduralSky&) = delete;
        ProceduralSky& operator=(const ProceduralSky&) = delete;

        void initialize(resources::ResourceManager& rm);
        void shutdown();

        void set_params(const SkyParams& params) {
            m_params = params;
        }

        [[nodiscard]] const SkyParams& params() const {
            return m_params;
        }

        void draw(graphics::DeviceContext& context, u32 view_id) const;

        [[nodiscard]] bool is_initialized() const {
            return m_initialized;
        }

      private:
        static constexpr int GRID = 32;

        void build_grid();

        resources::ResourceManager* m_rm = nullptr;
        graphics::ResourceHandle<graphics::Shader> m_shader;

        std::vector<float> m_vertices;
        std::vector<u16> m_indices;

        SkyParams m_params{};
        bool m_initialized = false;
    };
} // namespace star::rendering
