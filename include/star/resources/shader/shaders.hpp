#pragma once

#include <bgfx/embedded_shader.h>
#ifdef STAR_PLATFORM_LINUX
    // idk why but on linux dxbc(direct x) is defined by bgfx even tho its not supported
    #undef BGFX_EMBEDDED_SHADER_DXBC
    #define BGFX_EMBEDDED_SHADER_DXBC(...)
#endif

#include <essl/f_atmosphere.sc.bin.h>
#include <essl/f_debug.sc.bin.h>
#include <essl/f_imgui.sc.bin.h>
#include <essl/f_material.sc.bin.h>
#include <essl/f_simple.sc.bin.h>
#include <essl/v_atmosphere.sc.bin.h>
#include <essl/v_debug.sc.bin.h>
#include <essl/v_imgui.sc.bin.h>
#include <essl/v_material.sc.bin.h>
#include <essl/v_simple.sc.bin.h>
#include <glsl/f_atmosphere.sc.bin.h>
#include <glsl/f_debug.sc.bin.h>
#include <glsl/f_imgui.sc.bin.h>
#include <glsl/f_material.sc.bin.h>
#include <glsl/f_simple.sc.bin.h>
#include <glsl/v_atmosphere.sc.bin.h>
#include <glsl/v_debug.sc.bin.h>
#include <glsl/v_imgui.sc.bin.h>
#include <glsl/v_material.sc.bin.h>
#include <glsl/v_simple.sc.bin.h>
#include <spirv/f_atmosphere.sc.bin.h>
#include <spirv/f_debug.sc.bin.h>
#include <spirv/f_imgui.sc.bin.h>
#include <spirv/f_material.sc.bin.h>
#include <spirv/f_simple.sc.bin.h>
#include <spirv/v_atmosphere.sc.bin.h>
#include <spirv/v_debug.sc.bin.h>
#include <spirv/v_imgui.sc.bin.h>
#include <spirv/v_material.sc.bin.h>
#include <spirv/v_simple.sc.bin.h>

#ifdef STAR_PLATFORM_WINDOWS
    #include <dx10/f_atmosphere.sc.bin.h>
    #include <dx10/f_debug.sc.bin.h>
    #include <dx10/f_imgui.sc.bin.h>
    #include <dx10/f_material.sc.bin.h>
    #include <dx10/f_simple.sc.bin.h>
    #include <dx10/v_atmosphere.sc.bin.h>
    #include <dx10/v_debug.sc.bin.h>
    #include <dx10/v_imgui.sc.bin.h>
    #include <dx10/v_material.sc.bin.h>
    #include <dx10/v_simple.sc.bin.h>
    #include <dx11/f_atmosphere.sc.bin.h>
    #include <dx11/f_debug.sc.bin.h>
    #include <dx11/f_imgui.sc.bin.h>
    #include <dx11/f_material.sc.bin.h>
    #include <dx11/f_simple.sc.bin.h>
    #include <dx11/v_atmosphere.sc.bin.h>
    #include <dx11/v_debug.sc.bin.h>
    #include <dx11/v_imgui.sc.bin.h>
    #include <dx11/v_material.sc.bin.h>
    #include <dx11/v_simple.sc.bin.h>
#endif
#ifdef STAR_PLATFORM_MACOS
    #include <mtl/f_atmosphere.sc.bin.h>
    #include <mtl/f_debug.sc.bin.h>
    #include <mtl/f_imgui.sc.bin.h>
    #include <mtl/f_material.sc.bin.h>
    #include <mtl/f_simple.sc.bin.h>
    #include <mtl/v_atmosphere.sc.bin.h>
    #include <mtl/v_debug.sc.bin.h>
    #include <mtl/v_imgui.sc.bin.h>
    #include <mtl/v_material.sc.bin.h>
    #include <mtl/v_simple.sc.bin.h>
#endif

const bgfx::EmbeddedShader k_simple_vs = BGFX_EMBEDDED_SHADER(v_simple);
const bgfx::EmbeddedShader k_simple_fs = BGFX_EMBEDDED_SHADER(f_simple);

const bgfx::EmbeddedShader k_material_vs = BGFX_EMBEDDED_SHADER(v_material);
const bgfx::EmbeddedShader k_material_fs = BGFX_EMBEDDED_SHADER(f_material);

const bgfx::EmbeddedShader k_imgui_fs = BGFX_EMBEDDED_SHADER(f_imgui);
const bgfx::EmbeddedShader k_imgui_vs = BGFX_EMBEDDED_SHADER(v_imgui);

const bgfx::EmbeddedShader k_atmosphere_vs = BGFX_EMBEDDED_SHADER(v_atmosphere);
const bgfx::EmbeddedShader k_atmosphere_fs = BGFX_EMBEDDED_SHADER(f_atmosphere);

const bgfx::EmbeddedShader k_debug_vs = BGFX_EMBEDDED_SHADER(v_debug);
const bgfx::EmbeddedShader k_debug_fs = BGFX_EMBEDDED_SHADER(f_debug);
