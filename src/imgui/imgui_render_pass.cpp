#include "star/imgui/imgui_render_pass.hpp"

#include <imgui.h>

#include "star/core/logger.hpp"
#include "star/graphics/device_context.hpp"
#include "star/platform/window.hpp"

namespace star::rendering {
    ImGuiRenderPass::ImGuiRenderPass(std::unique_ptr<platform::IImGuiPlatformBackend> platform_backend,
                                     std::unique_ptr<platform::IImGuiRenderer> renderer)
        : m_platform_backend(std::move(platform_backend)), m_renderer(std::move(renderer)) {
        m_context = ImGui::CreateContext();
        ImGui::SetCurrentContext(m_context);

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        setup_style();

        STAR_LOG_INFO(LogCategory::Platform, "ImGui context created - awaiting window setup");
    }

    void ImGuiRenderPass::set_window(const platform::Window* window) {
        if (m_initialized) {
            STAR_LOG_WARN(LogCategory::Platform, "ImGui already initialized");
            return;
        }

        if (!window) {
            STAR_LOG_ERROR(LogCategory::Platform, "Invalid window for ImGui initialization");
            return;
        }

        if (m_platform_backend && !m_platform_backend->initialize(window->handle())) {
            STAR_LOG_ERROR(LogCategory::Platform, "Failed to initialize ImGui platform backend");
            return;
        }

        apply_dpi_scale(window->pixel_density());

        if (m_renderer && !m_renderer->initialize()) {
            STAR_LOG_ERROR(LogCategory::Platform, "Failed to initialize ImGui renderer");
            if (m_platform_backend) {
                m_platform_backend->shutdown();
            }
            return;
        }

        m_initialized = true;
        STAR_LOG_INFO(LogCategory::Platform, "ImGui system fully initialized");
    }

    ImGuiRenderPass::~ImGuiRenderPass() {
        if (!m_initialized) {
            return;
        }

        STAR_LOG_INFO(LogCategory::Platform, "Shutting down ImGui system...");

        if (m_renderer) {
            m_renderer->shutdown();
            m_renderer.reset();
        }

        if (m_platform_backend) {
            m_platform_backend->shutdown();
            m_platform_backend.reset();
        }

        if (m_context) {
            ImGui::DestroyContext(m_context);
            m_context = nullptr;
        }

        m_initialized = false;

        Super::set_enabled(false);
    }

    void ImGuiRenderPass::pre_render(const FrameContext& /*frame*/) {
        if (!m_initialized) {
            return;
        }

        if (m_platform_backend) {
            m_platform_backend->new_frame();
        }
        ImGui::NewFrame();

        for (const auto& callback : m_imgui_callbacks) {
            if (callback) {
                callback();
            }
        }

        ImGui::SetCurrentContext(m_context);
        ImGui::Render();
    }

    void ImGuiRenderPass::render(const RenderContext& ctx) {
        if (!m_initialized) {
            return;
        }

        if (m_renderer) {
            m_renderer->render(ctx.view_id);
        }
    }

    void ImGuiRenderPass::post_render(const FrameContext& /*frame*/) {
        if (!m_initialized) {
            return;
        }

        ImGui::SetCurrentContext(m_context);
    }

    void ImGuiRenderPass::on_resize(const u32 width, const u32 height) {
        if (!m_initialized) {
            return;
        }

        if (m_renderer) {
            m_renderer->reset(width, height);
        }
    }

    void ImGuiRenderPass::apply_dpi_scale(const f32 density) {
        ImGui::SetCurrentContext(m_context);

        ImGuiIO& io = ImGui::GetIO();
        if (io.Fonts->Sources.empty()) {
            io.Fonts->AddFontDefault();
        }

        for (ImFontConfig& source : io.Fonts->Sources) {
            source.RasterizerDensity = density;
        }

        STAR_LOG_INFO(LogCategory::Platform, "ImGui font rasterizer density set to {} ({} sources)", density,
                      io.Fonts->Sources.Size);
    }

    void ImGuiRenderPass::setup_style() {
        ImGui::StyleColorsDark();
    }

} // namespace star::rendering
