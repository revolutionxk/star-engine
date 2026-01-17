#include "star/core/common.hpp"
#include "star/render/material.hpp"
#include "star/graphics/shaders.hpp"
#include "star/render/texture.hpp"

namespace star {
    Material::Material() {
        update_state();
    }

    Material::~Material() = default;

    Material::Material(Material &&other) noexcept
        : _shader(std::move(other._shader))
          , _state(other._state)
          , _depth_test(other._depth_test)
          , _depth_write(other._depth_write)
          , _depth_func(other._depth_func)
          , _blend_mode(other._blend_mode)
          , _cull_mode(other._cull_mode) {
    }

    Material &Material::operator=(Material &&other) noexcept {
        if (this != &other) {
            _shader = std::move(other._shader);
            _state = other._state;
            _depth_test = other._depth_test;
            _depth_write = other._depth_write;
            _depth_func = other._depth_func;
            _blend_mode = other._blend_mode;
            _cull_mode = other._cull_mode;
        }
        return *this;
    }

    bool Material::set_shader(Shader &&shader) {
        if (!shader.is_valid()) {
            return false;
        }

        _shader = std::move(shader);
        return true;
    }

    bool Material::set_texture(const std::string &sampler_name, bgfx::TextureHandle texture, uint32_t flags) {
        TextureSampler *sampler = _shader.get_sampler(sampler_name);
        if (!sampler) {
            return false;
        }

        sampler->handle = texture;
        sampler->flags = flags;
        return true;
    }

    bool Material::set_uniform(const std::string &name, const glm::vec4 &value) {
        const ShaderUniform *uniform = _shader.get_uniform(name);
        if (!uniform) {
            spdlog::error("Material::set_uniform - Uniform '{}' not found in shader!", name);
            return false;
        }
        if (uniform->type != bgfx::UniformType::Vec4) {
            spdlog::error("Material::set_uniform - Uniform '{}' type mismatch (expected Vec4)", name);
            return false;
        }
        if (!bgfx::isValid(uniform->handle)) {
            spdlog::error("Material::set_uniform - Uniform '{}' handle is invalid!", name);
            return false;
        }
        bgfx::setUniform(uniform->handle, &value);
        return true;
    }

    bool Material::set_uniform(const std::string &name, const glm::mat4 &value) {
        const ShaderUniform *uniform = _shader.get_uniform(name);
        if (!uniform) {
            spdlog::error("Material::set_uniform - Uniform '{}' not found in shader!", name);
            return false;
        }
        if (uniform->type != bgfx::UniformType::Mat4) {
            spdlog::error("Material::set_uniform - Uniform '{}' type mismatch (expected Mat4)", name);
            return false;
        }
        if (!bgfx::isValid(uniform->handle)) {
            spdlog::error("Material::set_uniform - Uniform '{}' handle is invalid!", name);
            return false;
        }
        bgfx::setUniform(uniform->handle, &value);
        return true;
    }

    bool Material::set_uniform(const std::string &name, const float *data, uint16_t count) {
        const ShaderUniform *uniform = _shader.get_uniform(name);
        if (!uniform) {
            spdlog::error("Material::set_uniform - Uniform '{}' not found in shader!", name);
            return false;
        }
        if (!bgfx::isValid(uniform->handle)) {
            spdlog::error("Material::set_uniform - Uniform '{}' handle is invalid!", name);
            return false;
        }

        bgfx::setUniform(uniform->handle, data, count);
        return true;
    }

    void Material::set_blend_mode(BlendMode mode) {
        _blend_mode = mode;
        update_state();
    }

    BlendMode Material::get_blend_mode() const {
        return _blend_mode;
    }

    void Material::set_depth_test(bool enabled) {
        _depth_test = enabled;
        update_state();
    }

    bool Material::get_depth_test() const {
        return _depth_test;
    }

    void Material::set_depth_write(bool enabled) {
        _depth_write = enabled;
        update_state();
    }

    bool Material::get_depth_write() const {
        return _depth_write;
    }

    void Material::set_depth_function(DepthFunc func) {
        _depth_func = func;
        update_state();
    }

    DepthFunc Material::get_depth_function() const {
        return _depth_func;
    }

    void Material::set_cull_mode(CullMode mode) {
        _cull_mode = mode;
        update_state();
    }

    CullMode Material::get_cull_mode() const {
        return _cull_mode;
    }

    void Material::bind(bgfx::Encoder *encoder, uint8_t view_id) const {
        if (!_shader.is_valid()) {
            spdlog::warn("Material::bind - Invalid shader");
            return;
        }

        encoder->setState(_state);

        for (const auto &sampler: _shader._samplers | std::views::values) {
            if (bgfx::isValid(sampler.handle)) {
                encoder->setTexture(sampler.stage, sampler.sampler, sampler.handle, sampler.flags);
            }
        }

        encoder->submit(view_id, _shader.get_handle());
    }

    uint32_t Material::generate_sort_key() const {
        const uint32_t type_key = static_cast<uint32_t>(get_type()) << 24;
        const uint32_t blend_key = static_cast<uint32_t>(_blend_mode) << 20;
        const uint32_t shader_key = _shader.is_valid() ? static_cast<uint32_t>(_shader.get_handle().idx & 0xFFFFF) : 0;

        return type_key | blend_key | shader_key;
    }

    void Material::update_state() {
        _state = BGFX_STATE_WRITE_RGB | BGFX_STATE_MSAA;

        if (_depth_test) {
            switch (_depth_func) {
                case DepthFunc::Less:
                    _state |= BGFX_STATE_DEPTH_TEST_LESS;
                    break;
                case DepthFunc::LessEqual:
                    _state |= BGFX_STATE_DEPTH_TEST_LEQUAL;
                    break;
                case DepthFunc::Equal:
                    _state |= BGFX_STATE_DEPTH_TEST_EQUAL;
                    break;
                case DepthFunc::GreaterEqual:
                    _state |= BGFX_STATE_DEPTH_TEST_GEQUAL;
                    break;
                case DepthFunc::Greater:
                    _state |= BGFX_STATE_DEPTH_TEST_GREATER;
                    break;
                case DepthFunc::NotEqual:
                    _state |= BGFX_STATE_DEPTH_TEST_NOTEQUAL;
                    break;
                case DepthFunc::Always:
                    _state |= BGFX_STATE_DEPTH_TEST_ALWAYS;
                    break;
                case DepthFunc::Never:
                    _state |= BGFX_STATE_DEPTH_TEST_NEVER;
                    break;
            }
        }

        if (_depth_write) {
            _state |= BGFX_STATE_WRITE_Z;
        }

        switch (_cull_mode) {
            case CullMode::None:
                break;
            case CullMode::CW:
                _state |= BGFX_STATE_CULL_CW;
                break;
            case CullMode::CCW:
                _state |= BGFX_STATE_CULL_CCW;
                break;
        }

        switch (_blend_mode) {
            case BlendMode::Opaque:
                break;
            case BlendMode::Alpha:
                _state |= BGFX_STATE_BLEND_ALPHA;
                break;
            case BlendMode::Additive:
                _state |= BGFX_STATE_BLEND_ADD;
                break;
            case BlendMode::Multiply:
                _state |= BGFX_STATE_BLEND_MULTIPLY;
                break;
        }
    }

    UnlitMaterial::UnlitMaterial() {
        _shader.load(k_simple_vs, k_simple_fs);
    }

    UnlitMaterial::~UnlitMaterial() = default;

    void UnlitMaterial::set_color(const glm::vec4 &color) {
        _color = color;
    }

    glm::vec4 UnlitMaterial::get_color() const {
        return _color;
    }

    StandardMaterial::StandardMaterial() {
    }

    StandardMaterial::~StandardMaterial() = default;

    void StandardMaterial::set_base_color(const glm::vec4 &color) {
        _base_color = color;
        set_uniform("u_baseColor", _base_color);
    }

    glm::vec4 StandardMaterial::get_base_color() const {
        return _base_color;
    }

    void StandardMaterial::set_metallic(float value) {
        _metallic = glm::clamp(value, 0.0f, 1.0f);
        glm::vec4 params(_metallic, _roughness, 0.0f, 0.0f);
        set_uniform("u_materialParams", params);
    }

    float StandardMaterial::get_metallic() const {
        return _metallic;
    }

    void StandardMaterial::set_roughness(float value) {
        _roughness = glm::clamp(value, 0.0f, 1.0f);
        glm::vec4 params(_metallic, _roughness, 0.0f, 0.0f);
        set_uniform("u_materialParams", params);
    }

    float StandardMaterial::get_roughness() const {
        return _roughness;
    }

    void StandardMaterial::set_emissive(const glm::vec3 &value) {
        _emissive = value;
        set_uniform("u_emissive", glm::vec4(_emissive, 1.0f));
    }

    glm::vec3 StandardMaterial::get_emissive() const {
        return _emissive;
    }

    MaterialComponent::MaterialComponent() = default;
    MaterialComponent::~MaterialComponent() = default;
    MaterialComponent::MaterialComponent(MaterialComponent&&) noexcept = default;
    MaterialComponent& MaterialComponent::operator=(MaterialComponent&&) noexcept = default;

    void MaterialComponent::set_material(std::shared_ptr<Material> material) {
        _material = std::move(material);
    }

    std::shared_ptr<Material> MaterialComponent::get_material() const {
        return _material;
    }

    void MaterialComponent::init(App& app) {
        IAppComponent::init(app);
    }

    void MaterialComponent::shutdown() {
        _material.reset();
    }

    void MaterialComponent::update(float /*delta_time*/) {

    }

    void MaterialComponent::render() {

    }

    bgfx::ViewId MaterialComponent::render_reset(bgfx::ViewId view_id) {
        return view_id;
    }
}
