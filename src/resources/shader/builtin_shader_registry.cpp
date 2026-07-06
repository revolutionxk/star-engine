#include "builtin_shader_registry.hpp"

#ifdef STAR_PLATFORM_LINUX
    // bgfx defines DXBC even on platforms that don't support it
    #undef BGFX_EMBEDDED_SHADER_DXBC
    #define BGFX_EMBEDDED_SHADER_DXBC(...)
#endif

#include <essl/f_atmosphere.sc.bin.h>
#include <essl/f_debug.sc.bin.h>
#include <essl/f_imgui.sc.bin.h>
#include <essl/f_material.sc.bin.h>
#include <essl/f_blur.sc.bin.h>
#include <essl/f_brightpass.sc.bin.h>
#include <essl/f_fxaa.sc.bin.h>
#include <essl/f_ssao.sc.bin.h>
#include <essl/f_ssaoblur.sc.bin.h>
#include <essl/f_ssr.sc.bin.h>
#include <essl/f_pick.sc.bin.h>
#include <essl/f_shadow.sc.bin.h>
#include <essl/f_simple.sc.bin.h>
#include <essl/f_tonemap.sc.bin.h>
#include <essl/v_atmosphere.sc.bin.h>
#include <essl/v_debug.sc.bin.h>
#include <essl/v_imgui.sc.bin.h>
#include <essl/v_material.sc.bin.h>
#include <essl/v_fullscreen.sc.bin.h>
#include <essl/v_pick.sc.bin.h>
#include <essl/v_shadow.sc.bin.h>
#include <essl/v_simple.sc.bin.h>
#include <glsl/f_atmosphere.sc.bin.h>
#include <glsl/f_debug.sc.bin.h>
#include <glsl/f_imgui.sc.bin.h>
#include <glsl/f_material.sc.bin.h>
#include <glsl/f_blur.sc.bin.h>
#include <glsl/f_brightpass.sc.bin.h>
#include <glsl/f_fxaa.sc.bin.h>
#include <glsl/f_ssao.sc.bin.h>
#include <glsl/f_ssaoblur.sc.bin.h>
#include <glsl/f_ssr.sc.bin.h>
#include <glsl/f_pick.sc.bin.h>
#include <glsl/f_shadow.sc.bin.h>
#include <glsl/f_simple.sc.bin.h>
#include <glsl/f_tonemap.sc.bin.h>
#include <glsl/v_atmosphere.sc.bin.h>
#include <glsl/v_debug.sc.bin.h>
#include <glsl/v_imgui.sc.bin.h>
#include <glsl/v_material.sc.bin.h>
#include <glsl/v_fullscreen.sc.bin.h>
#include <glsl/v_pick.sc.bin.h>
#include <glsl/v_shadow.sc.bin.h>
#include <glsl/v_simple.sc.bin.h>
#include <spirv/f_atmosphere.sc.bin.h>
#include <spirv/f_debug.sc.bin.h>
#include <spirv/f_imgui.sc.bin.h>
#include <spirv/f_material.sc.bin.h>
#include <spirv/f_blur.sc.bin.h>
#include <spirv/f_brightpass.sc.bin.h>
#include <spirv/f_fxaa.sc.bin.h>
#include <spirv/f_ssao.sc.bin.h>
#include <spirv/f_ssaoblur.sc.bin.h>
#include <spirv/f_ssr.sc.bin.h>
#include <spirv/f_pick.sc.bin.h>
#include <spirv/f_shadow.sc.bin.h>
#include <spirv/f_simple.sc.bin.h>
#include <spirv/f_tonemap.sc.bin.h>
#include <spirv/v_atmosphere.sc.bin.h>
#include <spirv/v_debug.sc.bin.h>
#include <spirv/v_imgui.sc.bin.h>
#include <spirv/v_material.sc.bin.h>
#include <spirv/v_fullscreen.sc.bin.h>
#include <spirv/v_pick.sc.bin.h>
#include <spirv/v_shadow.sc.bin.h>
#include <spirv/v_simple.sc.bin.h>

#ifdef STAR_PLATFORM_WINDOWS
    #include <dx10/f_atmosphere.sc.bin.h>
    #include <dx10/f_debug.sc.bin.h>
    #include <dx10/f_imgui.sc.bin.h>
    #include <dx10/f_material.sc.bin.h>
    #include <dx10/f_blur.sc.bin.h>
    #include <dx10/f_brightpass.sc.bin.h>
    #include <dx10/f_fxaa.sc.bin.h>
    #include <dx10/f_ssao.sc.bin.h>
    #include <dx10/f_ssaoblur.sc.bin.h>
    #include <dx10/f_ssr.sc.bin.h>
    #include <dx10/f_pick.sc.bin.h>
    #include <dx10/f_shadow.sc.bin.h>
    #include <dx10/f_simple.sc.bin.h>
    #include <dx10/f_tonemap.sc.bin.h>
    #include <dx10/v_atmosphere.sc.bin.h>
    #include <dx10/v_debug.sc.bin.h>
    #include <dx10/v_imgui.sc.bin.h>
    #include <dx10/v_material.sc.bin.h>
    #include <dx10/v_fullscreen.sc.bin.h>
    #include <dx10/v_pick.sc.bin.h>
    #include <dx10/v_shadow.sc.bin.h>
    #include <dx10/v_simple.sc.bin.h>
    #include <dx11/f_atmosphere.sc.bin.h>
    #include <dx11/f_debug.sc.bin.h>
    #include <dx11/f_imgui.sc.bin.h>
    #include <dx11/f_material.sc.bin.h>
    #include <dx11/f_blur.sc.bin.h>
    #include <dx11/f_brightpass.sc.bin.h>
    #include <dx11/f_fxaa.sc.bin.h>
    #include <dx11/f_ssao.sc.bin.h>
    #include <dx11/f_ssaoblur.sc.bin.h>
    #include <dx11/f_ssr.sc.bin.h>
    #include <dx11/f_pick.sc.bin.h>
    #include <dx11/f_shadow.sc.bin.h>
    #include <dx11/f_simple.sc.bin.h>
    #include <dx11/f_tonemap.sc.bin.h>
    #include <dx11/v_atmosphere.sc.bin.h>
    #include <dx11/v_debug.sc.bin.h>
    #include <dx11/v_imgui.sc.bin.h>
    #include <dx11/v_material.sc.bin.h>
    #include <dx11/v_fullscreen.sc.bin.h>
    #include <dx11/v_pick.sc.bin.h>
    #include <dx11/v_shadow.sc.bin.h>
    #include <dx11/v_simple.sc.bin.h>
#endif
#ifdef STAR_PLATFORM_MACOS
    #include <mtl/f_atmosphere.sc.bin.h>
    #include <mtl/f_debug.sc.bin.h>
    #include <mtl/f_imgui.sc.bin.h>
    #include <mtl/f_material.sc.bin.h>
    #include <mtl/f_blur.sc.bin.h>
    #include <mtl/f_brightpass.sc.bin.h>
    #include <mtl/f_fxaa.sc.bin.h>
    #include <mtl/f_ssao.sc.bin.h>
    #include <mtl/f_ssaoblur.sc.bin.h>
    #include <mtl/f_ssr.sc.bin.h>
    #include <mtl/f_pick.sc.bin.h>
    #include <mtl/f_shadow.sc.bin.h>
    #include <mtl/f_simple.sc.bin.h>
    #include <mtl/f_tonemap.sc.bin.h>
    #include <mtl/v_atmosphere.sc.bin.h>
    #include <mtl/v_debug.sc.bin.h>
    #include <mtl/v_imgui.sc.bin.h>
    #include <mtl/v_material.sc.bin.h>
    #include <mtl/v_fullscreen.sc.bin.h>
    #include <mtl/v_pick.sc.bin.h>
    #include <mtl/v_shadow.sc.bin.h>
    #include <mtl/v_simple.sc.bin.h>
#endif

namespace star::resources::detail {

    namespace {
        const bgfx::EmbeddedShader k_simple_vs = BGFX_EMBEDDED_SHADER(v_simple);
        const bgfx::EmbeddedShader k_simple_fs = BGFX_EMBEDDED_SHADER(f_simple);
        const bgfx::EmbeddedShader k_material_vs = BGFX_EMBEDDED_SHADER(v_material);
        const bgfx::EmbeddedShader k_material_fs = BGFX_EMBEDDED_SHADER(f_material);
        const bgfx::EmbeddedShader k_imgui_vs = BGFX_EMBEDDED_SHADER(v_imgui);
        const bgfx::EmbeddedShader k_imgui_fs = BGFX_EMBEDDED_SHADER(f_imgui);
        const bgfx::EmbeddedShader k_atmosphere_vs = BGFX_EMBEDDED_SHADER(v_atmosphere);
        const bgfx::EmbeddedShader k_atmosphere_fs = BGFX_EMBEDDED_SHADER(f_atmosphere);
        const bgfx::EmbeddedShader k_debug_vs = BGFX_EMBEDDED_SHADER(v_debug);
        const bgfx::EmbeddedShader k_debug_fs = BGFX_EMBEDDED_SHADER(f_debug);
        const bgfx::EmbeddedShader k_pick_vs = BGFX_EMBEDDED_SHADER(v_pick);
        const bgfx::EmbeddedShader k_pick_fs = BGFX_EMBEDDED_SHADER(f_pick);
        const bgfx::EmbeddedShader k_shadow_vs = BGFX_EMBEDDED_SHADER(v_shadow);
        const bgfx::EmbeddedShader k_shadow_fs = BGFX_EMBEDDED_SHADER(f_shadow);
        const bgfx::EmbeddedShader k_fullscreen_vs = BGFX_EMBEDDED_SHADER(v_fullscreen);
        const bgfx::EmbeddedShader k_tonemap_fs = BGFX_EMBEDDED_SHADER(f_tonemap);
        const bgfx::EmbeddedShader k_brightpass_fs = BGFX_EMBEDDED_SHADER(f_brightpass);
        const bgfx::EmbeddedShader k_blur_fs = BGFX_EMBEDDED_SHADER(f_blur);
        const bgfx::EmbeddedShader k_fxaa_fs = BGFX_EMBEDDED_SHADER(f_fxaa);
        const bgfx::EmbeddedShader k_ssao_fs = BGFX_EMBEDDED_SHADER(f_ssao);
        const bgfx::EmbeddedShader k_ssaoblur_fs = BGFX_EMBEDDED_SHADER(f_ssaoblur);
        const bgfx::EmbeddedShader k_ssr_fs = BGFX_EMBEDDED_SHADER(f_ssr);
    } // namespace

    EmbeddedShaderPair embedded_pair_for(const BuiltinShader id) noexcept {
        switch (id) {
            case BuiltinShader::Simple:
                return {&k_simple_vs, &k_simple_fs};
            case BuiltinShader::Material:
                return {&k_material_vs, &k_material_fs};
            case BuiltinShader::ImGui:
                return {&k_imgui_vs, &k_imgui_fs};
            case BuiltinShader::Atmosphere:
                return {&k_atmosphere_vs, &k_atmosphere_fs};
            case BuiltinShader::Debug:
                return {&k_debug_vs, &k_debug_fs};
            case BuiltinShader::Picking:
                return {&k_pick_vs, &k_pick_fs};
            case BuiltinShader::Shadow:
                return {&k_shadow_vs, &k_shadow_fs};
            case BuiltinShader::Tonemap:
                return {&k_fullscreen_vs, &k_tonemap_fs};
            case BuiltinShader::BloomBright:
                return {&k_fullscreen_vs, &k_brightpass_fs};
            case BuiltinShader::BloomBlur:
                return {&k_fullscreen_vs, &k_blur_fs};
            case BuiltinShader::Fxaa:
                return {&k_fullscreen_vs, &k_fxaa_fs};
            case BuiltinShader::Ssao:
                return {&k_fullscreen_vs, &k_ssao_fs};
            case BuiltinShader::SsaoBlur:
                return {&k_fullscreen_vs, &k_ssaoblur_fs};
            case BuiltinShader::Ssr:
                return {&k_fullscreen_vs, &k_ssr_fs};
        }
        return {nullptr, nullptr};
    }

    graphics::ResourceHandle<graphics::Shader> create_embedded_program(const BuiltinShader id,
                                                                       const std::string_view name) {
        const auto pair = embedded_pair_for(id);
        if (pair.vertex == nullptr || pair.fragment == nullptr) {
            return {};
        }

        const auto renderer_type = bgfx::getCaps()->rendererType;

        const auto vs = bgfx::createEmbeddedShader(pair.vertex, renderer_type, pair.vertex->name);
        if (!bgfx::isValid(vs)) {
            return {};
        }

        const auto fs = bgfx::createEmbeddedShader(pair.fragment, renderer_type, pair.fragment->name);
        if (!bgfx::isValid(fs)) {
            bgfx::destroy(vs);
            return {};
        }

        const auto program = bgfx::createProgram(vs, fs, true);
        if (!bgfx::isValid(program)) {
            return {};
        }

        (void)name;
        return graphics::ResourceHandle<graphics::Shader>{program.idx, 0};
    }

} // namespace star::resources::detail
