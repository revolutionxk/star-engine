#include "render_settings_panel.hpp"

#include <imgui.h>

#include "../core/file_dialog.hpp"
#include "../editor_window.hpp"
#include "star/rendering/passes/shadow_pass.hpp"
#include "star/rendering/renderer.hpp"
#include "star/rendering/systems/render_system.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/texture/texture.hpp"

namespace star::editor {
    void RenderSettingsPanel::on_imgui_render() {
        if (!m_is_open)
            return;

        ImGui::Begin(m_name.c_str(), &m_is_open);

        auto* shadow_pass = m_editor_window->renderer().get_render_pass<rendering::ShadowPass>();
        if (shadow_pass) {
            ImGui::SeparatorText("Directional Shadows");

            auto& s = shadow_pass->settings();
            ImGui::Checkbox("Enabled", &s.enabled);

            ImGui::BeginDisabled(!s.enabled);
            ImGui::DragFloat("Depth Bias", &s.depth_bias, 0.0001f, 0.0f, 0.02f, "%.4f");
            ImGui::SetItemTooltip("Raise to remove shadow acne (stripes); lower if shadows detach (peter-panning).");
            ImGui::DragFloat("Coverage", &s.coverage, 1.0f, 5.0f, 200.0f, "%.0f");
            ImGui::SetItemTooltip("Orthographic half-size around the camera. Larger = more area but softer/blockier.");
            ImGui::EndDisabled();
        } else {
            ImGui::TextDisabled("Shadow pass unavailable");
        }

        {
            ImGui::SeparatorText("Post-Processing");

            auto& p = m_editor_window->renderer().post_settings();
            ImGui::Checkbox("Post-Processing Enabled", &p.enabled);
            ImGui::BeginDisabled(!p.enabled);
            ImGui::DragFloat("Exposure", &p.exposure, 0.01f, 0.05f, 8.0f, "%.2f");
            ImGui::SetItemTooltip("Overall scene brightness before tonemapping.");

            ImGui::Checkbox("TAA", &p.taa_enabled);
            ImGui::SetItemTooltip("Temporal anti-aliasing: subpixel jitter + reprojection. Cleaner than FXAA and "
                                  "denoises GTAO/SSR over time. Supersedes FXAA when on.");
            ImGui::BeginDisabled(!p.taa_enabled);
            ImGui::DragFloat("TAA Blend", &p.taa_blend, 0.005f, 0.0f, 0.98f, "%.3f");
            ImGui::SetItemTooltip("History weight: higher = smoother/steadier but more ghosting on motion.");
            ImGui::EndDisabled();

            ImGui::BeginDisabled(p.taa_enabled);
            ImGui::Checkbox("FXAA", &p.fxaa_enabled);
            ImGui::SetItemTooltip("Fast anti-aliasing (used only when TAA is off).");
            ImGui::EndDisabled();

            ImGui::Checkbox("SSAO", &p.ssao_enabled);
            ImGui::BeginDisabled(!p.ssao_enabled);
            ImGui::DragFloat("AO Strength", &p.ssao_strength, 0.02f, 0.0f, 2.0f, "%.2f");
            ImGui::SetItemTooltip("How much ambient occlusion darkens creases and contacts.");
            ImGui::DragFloat("AO Radius", &p.ssao_radius, 0.01f, 0.05f, 4.0f, "%.2f");
            ImGui::SetItemTooltip("Sample radius in world units. Larger = broader, softer occlusion.");
            ImGui::DragFloat("AO Power", &p.ssao_power, 0.05f, 0.1f, 6.0f, "%.2f");
            ImGui::SetItemTooltip("Contrast of the occlusion falloff.");
            ImGui::DragFloat("AO Fade Dist", &p.ssao_fade, 0.5f, 2.0f, 200.0f, "%.1f");
            ImGui::SetItemTooltip("View distance where AO fades out (stops grazing far geometry from turning noisy/dark).");
            ImGui::EndDisabled();

            ImGui::Checkbox("SSR", &p.ssr_enabled);
            ImGui::SetItemTooltip("Screen-space reflections: metallic surfaces reflect the scene's own geometry.");
            ImGui::BeginDisabled(!p.ssr_enabled);
            ImGui::DragFloat("SSR Intensity", &p.ssr_intensity, 0.02f, 0.0f, 2.0f, "%.2f");
            ImGui::DragFloat("SSR Distance", &p.ssr_max_distance, 0.1f, 0.5f, 50.0f, "%.1f");
            ImGui::SetItemTooltip("How far a reflection ray marches (view units).");
            ImGui::DragFloat("SSR Thickness", &p.ssr_thickness, 0.01f, 0.05f, 4.0f, "%.2f");
            ImGui::SetItemTooltip("Depth tolerance for a ray hit. Too small = gaps, too large = smears.");
            ImGui::EndDisabled();

            ImGui::Checkbox("Bloom", &p.bloom_enabled);
            ImGui::BeginDisabled(!p.bloom_enabled);
            ImGui::DragFloat("Threshold", &p.bloom_threshold, 0.01f, 0.0f, 10.0f, "%.2f");
            ImGui::SetItemTooltip("Brightness above which pixels start to glow. Lower = more bloom.");
            ImGui::DragFloat("Knee", &p.bloom_knee, 0.01f, 0.0f, 2.0f, "%.2f");
            ImGui::SetItemTooltip("Soft-knee width so the glow fades in smoothly around the threshold.");
            ImGui::DragFloat("Intensity", &p.bloom_intensity, 0.01f, 0.0f, 4.0f, "%.2f");
            ImGui::SetItemTooltip("How strongly the bloom is added back onto the image.");
            ImGui::EndDisabled();

            ImGui::EndDisabled();
        }

        if (auto* rs = m_editor_window->renderer().get_render_system()) {
            ImGui::SeparatorText("Environment (IBL)");
            auto env = rs->environment();
            ImGui::TextUnformatted(env.map.is_valid() ? "HDRI: loaded" : "HDRI: none");
            ImGui::SetItemTooltip("Load an equirectangular .hdr; metals/PBR surfaces reflect it (roughness blurs it).");

            if (ImGui::Button("Load HDRI...")) {
                m_editor_window->open_file_dialog(FileRequest::Environment);
            }
            if (env.map.is_valid()) {
                ImGui::SameLine();
                if (ImGui::Button("Clear Environment"))
                    rs->set_environment({});
                float intensity = env.intensity;
                if (ImGui::DragFloat("IBL Intensity", &intensity, 0.02f, 0.0f, 8.0f, "%.2f")) {
                    env.intensity = intensity;
                    rs->set_environment(env);
                }
            }
        }

        ImGui::End();
    }
} // namespace star::editor
