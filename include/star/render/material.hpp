#pragma once

#include "star/export.hpp"
#include <glm/glm.hpp>

#include "shader.hpp"
#include "star/app/app_component.hpp"

namespace star {
    class Shader;

    enum class MaterialType {
        Unlit,
        Standard,
        Custom
    };

    enum class BlendMode {
        Opaque,
        Alpha,
        Additive,
        Multiply
    };

    enum class DepthFunc {
        Less,
        LessEqual,
        Equal,
        GreaterEqual,
        Greater,
        NotEqual,
        Always,
        Never
    };

    enum class CullMode {
        None,
        CW,
        CCW
    };

    class STAR_EXPORT Material {
    public:
        Material();

        virtual ~Material();

        Material(const Material &) = delete;

        Material &operator=(const Material &) = delete;

        Material(Material &&other) noexcept;

        Material &operator=(Material &&other) noexcept;

        bool set_shader(Shader &&shader);

        bool set_texture(const std::string &sampler_name, bgfx::TextureHandle texture,
                         uint32_t flags = BGFX_SAMPLER_NONE);

        bool set_uniform(const std::string &name, const glm::vec4 &value);

        bool set_uniform(const std::string &name, const glm::mat4 &value);

        bool set_uniform(const std::string &name, const float *data, uint16_t count);

        void set_blend_mode(BlendMode mode);

        BlendMode get_blend_mode() const;

        void set_depth_test(bool enabled);

        bool get_depth_test() const;

        void set_depth_write(bool enabled);

        bool get_depth_write() const;

        void set_depth_function(DepthFunc func);

        DepthFunc get_depth_function() const;

        void set_cull_mode(CullMode mode);

        CullMode get_cull_mode() const;

        void bind(bgfx::Encoder *encoder, uint8_t view_id) const;

        virtual MaterialType get_type() const = 0;

        uint32_t generate_sort_key() const;

    protected:
        Shader _shader;

        uint64_t _state{BGFX_STATE_DEFAULT};
        bool _depth_test{true};
        bool _depth_write{true};
        DepthFunc _depth_func{DepthFunc::Less};
        BlendMode _blend_mode{BlendMode::Opaque};
        CullMode _cull_mode{CullMode::CCW};

        void update_state();
    };

    class STAR_EXPORT UnlitMaterial final : public Material {
    public:
        UnlitMaterial();

        ~UnlitMaterial() override;

        MaterialType get_type() const override { return MaterialType::Unlit; }

        void set_color(const glm::vec4 &color);

        glm::vec4 get_color() const;

    private:
        glm::vec4 _color{1.0f, 1.0f, 1.0f, 1.0f};
    };

    class STAR_EXPORT StandardMaterial final : public Material {
    public:
        StandardMaterial();

        ~StandardMaterial() override;

        MaterialType get_type() const override { return MaterialType::Standard; }

        void set_base_color(const glm::vec4 &color);

        glm::vec4 get_base_color() const;

        void set_metallic(float value);

        float get_metallic() const;

        void set_roughness(float value);

        float get_roughness() const;

        void set_emissive(const glm::vec3 &value);

        glm::vec3 get_emissive() const;

    private:
        glm::vec4 _base_color{1.0f, 1.0f, 1.0f, 1.0f};
        float _metallic{0.0f};
        float _roughness{0.5f};
        glm::vec3 _emissive{0.0f};
    };

    class STAR_EXPORT MaterialComponent : public ITypeAppComponent<MaterialComponent> {
    public:
        MaterialComponent();
        ~MaterialComponent() override;
        MaterialComponent(const MaterialComponent&) = delete;
        MaterialComponent& operator=(const MaterialComponent&) = delete;
        MaterialComponent(MaterialComponent&&) noexcept;
        MaterialComponent& operator=(MaterialComponent&&) noexcept;

        void set_material(std::shared_ptr<Material> material);
        std::shared_ptr<Material> get_material() const;

        void init(App& app) override;
        void shutdown() override;
        void update(float delta_time) override;
        void render() override;
        bgfx::ViewId render_reset(bgfx::ViewId view_id) override;

    private:
        std::shared_ptr<Material> _material;
    };

} // namespace star
