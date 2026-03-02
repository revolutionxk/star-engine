#pragma once

#include "imgui.h"
#include "star/core/reflection/reflect.hpp"
#include "star/math/math.hpp"

namespace star::editor::reflection {
    class ImGuiVisitor {
      public:
        template<typename FD, typename Value>
        void operator()(const FD& desc, Value& value) const {
            if constexpr (FD::template has_attr<star::reflection::attr::HideInEditor>())
                return;

            const bool is_ro = FD::template has_attr<star::reflection::attr::ReadOnly>();
            if (is_ro)
                ImGui::BeginDisabled();

            draw(desc, value);

            if (is_ro)
                ImGui::EndDisabled();

            if constexpr (FD::template has_attr<star::reflection::attr::Tooltip>()) {
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
                    if (auto tt = desc.template get_attr<star::reflection::attr::Tooltip>())
                        ImGui::SetTooltip("%s", tt->text.data());
                }
            }
        }

      private:
        template<typename FD>
        static void draw(const FD& d, f32& v) {
            const auto sp = d.template get_attr_or<star::reflection::attr::Speed>({0.1f}).value;
            const auto rng = d.template get_attr_or<star::reflection::attr::Range>({});
            ImGui::DragFloat(d.name.data(), &v, sp, rng.min, rng.max, "%.3f");
        }

        template<typename FD>
        static void draw(const FD& d, f64& v) {
            float fv = static_cast<float>(v);
            const auto sp = d.template get_attr_or<star::reflection::attr::Speed>({0.1f}).value;
            const auto rng = d.template get_attr_or<star::reflection::attr::Range>({});
            if (ImGui::DragFloat(d.name.data(), &fv, sp, rng.min, rng.max, "%.4f"))
                v = static_cast<f64>(fv);
        }

        template<typename FD>
        static void draw(const FD& d, i32& v) {
            const auto sp = d.template get_attr_or<star::reflection::attr::Speed>({1.f}).value;
            const auto rng = d.template get_attr_or<star::reflection::attr::Range>({});
            ImGui::DragInt(d.name.data(), &v, sp, static_cast<int>(rng.min), static_cast<int>(rng.max));
        }

        template<typename FD>
        static void draw(const FD& d, u32& v) {
            int iv = static_cast<int>(v);
            const auto sp = d.template get_attr_or<star::reflection::attr::Speed>({1.f}).value;
            const auto rng = d.template get_attr_or<star::reflection::attr::Range>({});
            if (ImGui::DragInt(d.name.data(), &iv, sp, 0, static_cast<int>(rng.max)))
                v = static_cast<u32>(iv);
        }

        template<typename FD>
        static void draw(const FD& d, u8& v) {
            int iv = v;
            const auto rng = d.template get_attr_or<star::reflection::attr::Range>({0.f, 255.f});
            if (ImGui::SliderInt(d.name.data(), &iv, static_cast<int>(rng.min), static_cast<int>(rng.max)))
                v = static_cast<u8>(iv);
        }

        template<typename FD>
        static void draw(const FD& d, bool& v) {
            ImGui::Checkbox(d.name.data(), &v);
        }

        template<typename FD>
        static void draw(const FD& d, std::string& v) {
            char buf[512]{};
            std::strncpy(buf, v.c_str(), sizeof(buf) - 1);
            if (ImGui::InputText(d.name.data(), buf, sizeof(buf)))
                v = buf;
        }

        template<typename FD>
        static void draw(const FD& d, Vector2& v) {
            const auto sp = d.template get_attr_or<star::reflection::attr::Speed>({0.1f}).value;
            const auto rng = d.template get_attr_or<star::reflection::attr::Range>({});
            ImGui::DragFloat2(d.name.data(), &v.x, sp, rng.min, rng.max, "%.3f");
        }

        template<typename FD>
        static void draw(const FD& d, Vector3& v) {
            if constexpr (FD::template has_attr<star::reflection::attr::Color>()) {
                ImGui::ColorEdit3(d.name.data(), v.data);
                return;
            }
            const auto sp = d.template get_attr_or<star::reflection::attr::Speed>({0.1f}).value;
            const auto rng = d.template get_attr_or<star::reflection::attr::Range>({});
            if (ImGui::DragFloat3(d.name.data(), v.data, sp, rng.min, rng.max, "%.3f")) {
                if constexpr (FD::template has_attr<star::reflection::attr::Normalized>()) {
                    if (const float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); len > 1e-5f) {
                        v.x /= len;
                        v.y /= len;
                        v.z /= len;
                    }
                }
            }
        }

        template<typename FD>
        static void draw(const FD& d, Vector4& v) {
            if constexpr (FD::template has_attr<star::reflection::attr::Color>()) {
                ImGui::ColorEdit4(d.name.data(), v.data);
                return;
            }
            const auto sp = d.template get_attr_or<star::reflection::attr::Speed>({0.1f}).value;
            const auto rng = d.template get_attr_or<star::reflection::attr::Range>({});
            ImGui::DragFloat4(d.name.data(), v.data, sp, rng.min, rng.max, "%.3f");
        }

        template<typename FD>
        static void draw(const FD& d, Quaternion& q) {
            auto euler = q.to_euler() * math::Constants<f32>::rad_to_deg;
            const auto sp = d.template get_attr_or<star::reflection::attr::Speed>({0.5f}).value;
            if (ImGui::DragFloat3(d.name.data(), euler.data, sp, 0.f, 0.f, "%.2f deg"))
                q = Quaternion::from_euler(radians(euler.x), radians(euler.y), radians(euler.z));
        }

        template<typename FD, typename Enum>
            requires std::is_enum_v<Enum> && (FD::has_enum_options())
        static void draw(const FD& d, Enum& v) {
            int current = static_cast<int>(v);
            d.visit_enum_options([&]<std::size_t N>(const star::reflection::attr::EnumOptions<N>& opts) {
                std::array<const char*, N> ptrs{};
                for (std::size_t i = 0; i < N; ++i)
                    ptrs[i] = opts.labels[i].data();
                if (ImGui::Combo(d.name.data(), &current, ptrs.data(), static_cast<int>(N)))
                    v = static_cast<Enum>(current);
            });
        }

        template<typename FD, typename Enum>
            requires std::is_enum_v<Enum> && (!FD::has_enum_options())
        static void draw(const FD& d, Enum& v) {
            int iv = static_cast<int>(v);
            if (ImGui::DragInt(d.name.data(), &iv, 1))
                v = static_cast<Enum>(iv);
        }
    };
} // namespace star::editor::reflection
