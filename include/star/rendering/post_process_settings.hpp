#pragma once

#include "star/core/types.hpp"

namespace star::rendering {
    enum class Tonemap : u8 {
        Aces,
        Agx,
        Reinhard,
        Gt7,
        None,
        Count,
    };

    inline constexpr const char* TONEMAP_NAMES[] = {"ACES", "AgX", "Reinhard", "GT7", "None"};

    struct PostProcessSettings {
        bool enabled = true;

        f32 exposure = 1.0f;
        Tonemap tonemap = Tonemap::Aces;

        bool bloom_enabled = true;
        f32 bloom_threshold = 1.1f;
        f32 bloom_knee = 0.5f;
        f32 bloom_intensity = 0.5f;
        f32 bloom_radius = 1.0f;

        bool fxaa_enabled = true;

        bool ssao_enabled = true;
        f32 ssao_radius = 0.6f;
        f32 ssao_power = 1.5f;
        f32 ssao_strength = 1.0f;
        f32 ssao_fade = 25.0f;

        bool ssr_enabled = true;
        f32 ssr_intensity = 1.0f;
        f32 ssr_max_distance = 8.0f;
        f32 ssr_thickness = 0.6f;

        bool taa_enabled = true;
        f32 taa_blend = 0.9f;

        [[nodiscard]] bool resolve_needs_fxaa() const noexcept {
            return enabled && fxaa_enabled && !taa_enabled;
        }
    };
} // namespace star::rendering
