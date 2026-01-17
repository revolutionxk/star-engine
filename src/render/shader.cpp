#include "star/core/common.hpp"
#include "star/render/shader.hpp"
#include "star/render/material.hpp"
#include "star/render/texture.hpp"

namespace star {
    Shader::Shader() = default;

    Shader::~Shader() {
        destroy();
    }

    Shader::Shader(Shader &&other) noexcept
        : _uniforms(std::move(other._uniforms))
          , _samplers(std::move(other._samplers))
          , _program(other._program) {
        other._program = BGFX_INVALID_HANDLE;
    }

    Shader &Shader::operator=(Shader &&other) noexcept {
        if (this != &other) {
            destroy();

            _program = other._program;
            _uniforms = std::move(other._uniforms);
            _samplers = std::move(other._samplers);

            other._program = BGFX_INVALID_HANDLE;
        }
        return *this;
    }

    bool Shader::load(const bgfx::Memory *vs_data, const bgfx::Memory *fs_data) {
        destroy();

        if (!vs_data || !fs_data) {
            spdlog::error("Shader::load - Invalid shader data");
            return false;
        }

        const bgfx::ShaderHandle vsh = bgfx::createShader(vs_data);
        const bgfx::ShaderHandle fsh = bgfx::createShader(fs_data);

        if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh)) {
            spdlog::error("Shader::load - Failed to create shader handles");
            return false;
        }

        _program = bgfx::createProgram(vsh, fsh, true);

        if (!is_valid()) {
            spdlog::error("Shader::load - Failed to create shader program");
            return false;
        }

        init_uniforms();

        return true;
    }

    bool Shader::load(const bgfx::EmbeddedShader &vs, const bgfx::EmbeddedShader &fs) {
        destroy();

        const bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(&vs, bgfx::getRendererType(), vs.name);
        const bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(&fs, bgfx::getRendererType(), fs.name);

        if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh)) {
            spdlog::error("Shader::load - Failed to create embedded shader handles");
            return false;
        }

        _program = bgfx::createProgram(vsh, fsh, true);

        if (!is_valid()) {
            spdlog::error("Shader::load - Failed to create embedded shader program");
            return false;
        }

        init_uniforms();

        return true;
    }

    bool Shader::is_valid() const {
        return bgfx::isValid(_program);
    }

    bgfx::ProgramHandle Shader::get_handle() const {
        return _program;
    }

    ShaderUniform *Shader::get_uniform(const std::string &name) {
        const auto it = _uniforms.find(name);
        if (it != _uniforms.end()) {
            return &it->second;
        }
        return nullptr;
    }

    TextureSampler *Shader::get_sampler(const std::string &name) {
        const auto it = _samplers.find(name);
        if (it != _samplers.end()) {
            return &it->second;
        }
        return nullptr;
    }

    void Shader::destroy() {
        _uniforms.clear();
        _samplers.clear();

        if (is_valid()) {
            bgfx::destroy(_program);
            _program = BGFX_INVALID_HANDLE;
        }
    }

    void Shader::init_uniforms() {
        _uniforms.emplace("u_color", ShaderUniform("u_color", bgfx::UniformType::Vec4));
        _uniforms.emplace("u_baseColor", ShaderUniform("u_baseColor", bgfx::UniformType::Vec4));
        _uniforms.emplace("u_emissive", ShaderUniform("u_emissive", bgfx::UniformType::Vec4));
        _uniforms.emplace("u_materialParams", ShaderUniform("u_materialParams", bgfx::UniformType::Vec4));
        _uniforms.emplace("u_camPos", ShaderUniform("u_camPos", bgfx::UniformType::Vec4));

        _samplers.emplace("s_texColor", TextureSampler("s_texColor", 0));
        _samplers.emplace("s_texNormal", TextureSampler("s_texNormal", 1));
        _samplers.emplace("s_texMetallicRoughness", TextureSampler("s_texMetallicRoughness", 2));
        _samplers.emplace("s_texEmissive", TextureSampler("s_texEmissive", 3));
    }

    ShaderUniform::ShaderUniform(const std::string &uniform_name, bgfx::UniformType::Enum uniform_type, uint16_t count)
        : name(uniform_name)
          , type(uniform_type)
          , num(count) {
        handle = bgfx::createUniform(name.c_str(), type, num);
    }

    ShaderUniform::~ShaderUniform() {
        destroy();
    }

    ShaderUniform::ShaderUniform(ShaderUniform &&other) noexcept
        : handle(other.handle)
          , name(std::move(other.name))
          , type(other.type)
          , num(other.num) {
        other.handle = BGFX_INVALID_HANDLE;
    }

    ShaderUniform &ShaderUniform::operator=(ShaderUniform &&other) noexcept {
        if (this != &other) {
            destroy();

            handle = other.handle;
            name = std::move(other.name);
            type = other.type;
            num = other.num;

            other.handle = BGFX_INVALID_HANDLE;
        }
        return *this;
    }

    bool ShaderUniform::is_valid() const {
        return bgfx::isValid(handle);
    }

    void ShaderUniform::destroy() {
        if (is_valid()) {
            bgfx::destroy(handle);
            handle = BGFX_INVALID_HANDLE;
        }
    }
}
