#include <algorithm>
#include <cmath>
#include <memory>

#include <imgui.h>

#include "star/application/app_window.hpp"
#include "star/application/application.hpp"
#include "star/application/layer.hpp"
#include "star/platform/input/input.hpp"
#include "star/platform/window.hpp"

using namespace star;

namespace {
    class ImGuiSampleLayer final : public application::Layer {
      public:
        explicit ImGuiSampleLayer(application::AppWindow* owner) : Layer("ImGuiSampleLayer"), m_owner(owner) {}

        void on_imgui_init() override {
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        }

        void update(const f32 delta_time) override {
            const auto& input = application::Application::instance().input_manager();
            const auto delta = input.mouse_delta();

            m_mouse_delta_max.x = std::max(m_mouse_delta_max.x, std::abs(delta.x));
            m_mouse_delta_max.y = std::max(m_mouse_delta_max.y, std::abs(delta.y));
        }

        void on_imgui_render() override {
            render_main_window();
            render_input_window();
            render_metrics_window();
        }

      private:
        void render_main_window() {
            ImGui::Begin("Star Engine - ImGui Sample");

            ImGui::TextUnformatted("ImGui runs as an overlay layer on top of the window render graph.");
            ImGui::Separator();

            ImGui::SliderFloat("Slider", &m_slider, 0.0f, 1.0f);
            ImGui::ColorEdit4("Color", &m_color.x);
            ImGui::Checkbox("Checkbox", &m_checked);

            ImGui::Spacing();

            if (ImGui::Button("Reset")) {
                m_slider = 0.5f;
                m_color = Vector4{0.3f, 0.55f, 0.9f, 1.0f};
                m_checked = false;
            }

            ImGui::End();
        }

        void render_input_window() {
            ImGui::Begin("Input");

            const auto& input = application::Application::instance().input_manager();
            const auto position = input.mouse_position();
            const auto delta = input.mouse_delta();

            ImGui::Text("Position: (%.1f, %.1f)", position.x, position.y);
            ImGui::Text("Delta: (%.1f, %.1f)", delta.x, delta.y);
            ImGui::Text("Delta Max: (%.1f, %.1f)", m_mouse_delta_max.x, m_mouse_delta_max.y);

            ImGui::Spacing();

            ImGui::Text("Left: %s", input.is_mouse_button_pressed(platform::MouseButton::Left) ? "DOWN" : "UP");
            ImGui::Text("Right: %s", input.is_mouse_button_pressed(platform::MouseButton::Right) ? "DOWN" : "UP");
            ImGui::Text("Middle: %s", input.is_mouse_button_pressed(platform::MouseButton::Middle) ? "DOWN" : "UP");

            ImGui::Spacing();

            ImGui::Text("Space: %s", input.is_key_pressed(platform::KeyCode::Space) ? "DOWN" : "UP");
            ImGui::Text("W A S D: %s %s %s %s", input.is_key_pressed(platform::KeyCode::W) ? "1" : "0",
                        input.is_key_pressed(platform::KeyCode::A) ? "1" : "0",
                        input.is_key_pressed(platform::KeyCode::S) ? "1" : "0",
                        input.is_key_pressed(platform::KeyCode::D) ? "1" : "0");

            ImGui::Spacing();

            if (ImGui::Button("Reset Max")) {
                m_mouse_delta_max = Vector2::zero();
            }

            ImGui::End();
        }

        void render_metrics_window() const {
            ImGui::Begin("Metrics");

            const auto& io = ImGui::GetIO();
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

            const auto size = m_owner->size();
            ImGui::Text("Window: %.0f x %.0f", size.x, size.y);

            ImGui::End();
        }

        application::AppWindow* m_owner = nullptr;

        Vector2 m_mouse_delta_max = Vector2::zero();
        Vector4 m_color{0.3f, 0.55f, 0.9f, 1.0f};
        f32 m_slider = 0.5f;
        bool m_checked = false;
    };

    class ImGuiSampleWindow final : public application::AppWindow {
      public:
        ImGuiSampleWindow() : AppWindow("Star Engine - ImGui Sample") {}

      protected:
        bool on_initialize() override {
            push_overlay(std::make_unique<ImGuiSampleLayer>(this));
            return true;
        }
    };

    class ImGuiSampleApp final : public application::Application {
      public:
        explicit ImGuiSampleApp(const application::CommandLineArgs& args) : Application(args) {}

      protected:
        bool on_initialize() override {
            if (!create_window<ImGuiSampleWindow>()) {
                STAR_LOG_ERROR(LogCategory::Application, "Failed to create sample window");
                return false;
            }
            return true;
        }
    };
} // namespace

STAR_RUN_APPLICATION(ImGuiSampleApp);
