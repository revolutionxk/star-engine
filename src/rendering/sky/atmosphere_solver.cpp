#include "star/rendering/sky/atmosphere_solver.hpp"

#include "star/core/common.hpp"
#include "star/rendering/render_scene.hpp"

namespace star::rendering {
    namespace {
        constexpr float ABCDE[5][3] = {
            {-0.2592f, -0.2608f, -1.4630f}, {0.0008f, 0.0092f, 0.4275f}, {0.2125f, 0.2102f, 5.3251f},
            {-0.8989f, -1.6537f, -2.5771f}, {0.0452f, 0.0529f, 0.3703f},
        };
        constexpr float ABCDE_t[5][3] = {
            {-0.0193f, -0.0167f, 0.1787f}, {-0.0665f, -0.0950f, -0.3554f}, {-0.0004f, -0.0079f, -0.0227f},
            {-0.0641f, -0.0441f, 0.1206f}, {-0.0033f, -0.0109f, -0.0670f},
        };

        constexpr std::pair<float, std::array<float, 3>> SKY_LUM_XYZ_TABLE[] = {
            {0.0f, {0.308f, 0.308f, 0.411f}},           {1.0f, {0.308f, 0.308f, 0.410f}},
            {2.0f, {0.301f, 0.301f, 0.402f}},           {3.0f, {0.287f, 0.287f, 0.382f}},
            {4.0f, {0.258f, 0.258f, 0.344f}},           {5.0f, {0.258f, 0.258f, 0.344f}},
            {7.0f, {0.962851f, 1.000000f, 1.747835f}},  {8.0f, {0.967787f, 1.000000f, 1.776762f}},
            {9.0f, {0.970173f, 1.000000f, 1.788413f}},  {10.0f, {0.971431f, 1.000000f, 1.794102f}},
            {11.0f, {0.972099f, 1.000000f, 1.797096f}}, {12.0f, {0.972385f, 1.000000f, 1.798389f}},
            {13.0f, {0.972361f, 1.000000f, 1.798278f}}, {14.0f, {0.972020f, 1.000000f, 1.796740f}},
            {15.0f, {0.971275f, 1.000000f, 1.793407f}}, {16.0f, {0.969885f, 1.000000f, 1.787078f}},
            {17.0f, {0.967216f, 1.000000f, 1.773758f}}, {18.0f, {0.961668f, 1.000000f, 1.739891f}},
            {20.0f, {0.264f, 0.264f, 0.352f}},          {21.0f, {0.264f, 0.264f, 0.352f}},
            {22.0f, {0.290f, 0.290f, 0.386f}},          {23.0f, {0.303f, 0.303f, 0.404f}},
        };

        constexpr std::pair<float, std::array<float, 3>> SUN_LUM_XYZ_TABLE[] = {
            {5.0f, {0.000000f, 0.000000f, 0.000000f}},     {7.0f, {12.703322f, 12.989393f, 9.100411f}},
            {8.0f, {13.202644f, 13.597814f, 11.524929f}},  {9.0f, {13.192974f, 13.597458f, 12.264488f}},
            {10.0f, {13.132943f, 13.535914f, 12.560032f}}, {11.0f, {13.088722f, 13.489535f, 12.692996f}},
            {12.0f, {13.067827f, 13.467483f, 12.745179f}}, {13.0f, {13.069653f, 13.469413f, 12.740822f}},
            {14.0f, {13.094319f, 13.495428f, 12.678066f}}, {15.0f, {13.142133f, 13.545483f, 12.526785f}},
            {16.0f, {13.201734f, 13.606017f, 12.188001f}}, {17.0f, {13.182774f, 13.572725f, 11.311157f}},
            {18.0f, {12.448635f, 12.672520f, 8.267771f}},  {20.0f, {0.000000f, 0.000000f, 0.000000f}},
        };

        std::array<float, 3> sample_table(const std::span<const std::pair<float, std::array<float, 3>>> table,
                                          const float t) {
            if (table.empty())
                return {0.f, 0.f, 0.f};

            if (t <= table.front().first)
                return table.front().second;
            if (t >= table.back().first)
                return table.back().second;

            for (size_t i = 1; i < table.size(); ++i) {
                if (t <= table[i].first) {
                    const float t0 = table[i - 1].first;
                    const float t1 = table[i].first;
                    const float alpha = (t - t0) / (t1 - t0);
                    const auto& v0 = table[i - 1].second;
                    const auto& v1 = table[i].second;
                    return {v0[0] + alpha * (v1[0] - v0[0]), v0[1] + alpha * (v1[1] - v0[1]),
                            v0[2] + alpha * (v1[2] - v0[2])};
                }
            }
            return table.back().second;
        }

        Vector3 xyz_to_rgb(const float x, const float y, const float z) {
            return {
                3.240479f * x - 1.537150f * y - 0.498535f * z,
                -0.969256f * x + 1.875991f * y + 0.041556f * z,
                0.055648f * x - 0.204043f * y + 1.057311f * z,
            };
        }

        void compute_perez_coeff(const float turbidity, float out[5][4]) {
            for (int i = 0; i < 5; ++i) {
                out[i][0] = ABCDE[i][0] + turbidity * ABCDE_t[i][0];
                out[i][1] = ABCDE[i][1] + turbidity * ABCDE_t[i][1];
                out[i][2] = ABCDE[i][2] + turbidity * ABCDE_t[i][2];
                out[i][3] = 0.0f;
            }
        }
    } // namespace

    AtmosphereSolution solve_atmosphere(const AtmosphereSnapshot& snapshot, const f32 elapsed_time) {
        AtmosphereSolution out;

        const float se = snapshot.sun_elevation;
        const float sa = snapshot.sun_azimuth;
        const Vector3 sun_dir{
            std::cos(se) * std::sin(sa),
            std::sin(se),
            std::cos(se) * std::cos(sa),
        };

        const float hour = 12.0f + se / (3.14159265f * 0.5f) * 6.0f;

        const auto sky_xyz = sample_table(SKY_LUM_XYZ_TABLE, hour);
        const auto sun_xyz = sample_table(SUN_LUM_XYZ_TABLE, hour);

        const Vector3 sky_rgb_raw = xyz_to_rgb(sky_xyz[0], sky_xyz[1], sky_xyz[2]);
        const Vector3 sun_rgb_raw = xyz_to_rgb(sun_xyz[0], sun_xyz[1], sun_xyz[2]);

        const auto normalize_color = [](const Vector3& c, const float target_peak) {
            const float m = std::max(std::max(c.x, c.y), std::max(c.z, 1e-4f));
            const float scale = target_peak / m;
            return Vector3{
                std::max(c.x, 0.0f) * scale,
                std::max(c.y, 0.0f) * scale,
                std::max(c.z, 0.0f) * scale,
            };
        };
        const Vector3 sky_rgb = normalize_color(sky_rgb_raw, 1.0f);
        const Vector3 sun_rgb = normalize_color(sun_rgb_raw, 1.0f);

        out.sky_params.sun_direction = sun_dir;
        out.sky_params.sky_luminance_xyz = {sky_xyz[0], sky_xyz[1], sky_xyz[2]};
        out.sky_params.sun_luminance = {sun_rgb.x, sun_rgb.y, sun_rgb.z};
        out.sky_params.exposition = 0.02f * snapshot.sun_intensity;
        out.sky_params.sun_size = 0.02f;
        out.sky_params.sun_bloom = 3.0f;
        out.sky_params.time = elapsed_time;
        compute_perez_coeff(snapshot.turbidity, out.sky_params.perez_coeff);

        out.lighting.valid = true;
        out.lighting.sun_direction = sun_dir;
        out.lighting.directional_dir = -sun_dir;
        out.lighting.sun_color_rgb = out.sky_params.sun_luminance;
        out.lighting.sun_intensity = snapshot.sun_intensity;
        out.lighting.sky_color_rgb = {std::max(sky_rgb.x, 0.0f), std::max(sky_rgb.y, 0.0f), std::max(sky_rgb.z, 0.0f)};

        return out;
    }
} // namespace star::rendering
