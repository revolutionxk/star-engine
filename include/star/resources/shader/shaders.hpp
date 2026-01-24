#pragma once

#include <essl/f_material.sc.bin.h>
#include <essl/f_simple.sc.bin.h>
#include <essl/v_material.sc.bin.h>
#include <essl/v_simple.sc.bin.h>
#include <glsl/f_material.sc.bin.h>
#include <glsl/f_simple.sc.bin.h>
#include <glsl/v_material.sc.bin.h>
#include <glsl/v_simple.sc.bin.h>
#include <spirv/f_material.sc.bin.h>
#include <spirv/f_simple.sc.bin.h>
#include <spirv/v_material.sc.bin.h>
#include <spirv/v_simple.sc.bin.h>

#include "bgfx/embedded_shader.h"

#if defined(_WIN32)
    #include <dx10/f_imgui.sc.bin.h>
    #include <dx10/f_material.sc.bin.h>
    #include <dx10/f_simple.sc.bin.h>
    #include <dx10/v_imgui.sc.bin.h>
    #include <dx10/v_material.sc.bin.h>
    #include <dx10/v_simple.sc.bin.h>
    #include <dx11/f_imgui.sc.bin.h>
    #include <dx11/f_material.sc.bin.h>
    #include <dx11/f_simple.sc.bin.h>
    #include <dx11/v_imgui.sc.bin.h>
    #include <dx11/v_material.sc.bin.h>
    #include <dx11/v_simple.sc.bin.h>
    #include <essl/f_imgui.sc.bin.h>
    #include <essl/v_imgui.sc.bin.h>
    #include <glsl/f_imgui.sc.bin.h>
    #include <glsl/v_imgui.sc.bin.h>
    #include <spirv/f_imgui.sc.bin.h>
    #include <spirv/v_imgui.sc.bin.h>
#endif
#if __APPLE__
    #include <mtl/f_imgui.sc.bin.h>
    #include <mtl/f_material.sc.bin.h>
    #include <mtl/f_simple.sc.bin.h>
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
