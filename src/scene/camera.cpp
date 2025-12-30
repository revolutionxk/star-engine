#include <algorithm>
#include "star/scene/camera.hpp"
#include "star/scene/transform.hpp"
#include "star/scene/scene.hpp"
#include "star/app/app.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <bgfx/bgfx.h>
#include <spdlog/spdlog.h>
#include <unordered_map>

#include "star/app/window.hpp"

namespace star {

    Camera::Camera() = default;

    Camera::~Camera() {
        shutdown();
    }

    void Camera::init(Scene &scene, App &app) {
        _scene = &scene;
        _app = &app;
        for (const auto &component: _components) {
            component->init(*this, scene, app);
        }
    }

    void Camera::shutdown() {
        for (auto it = _components.rbegin(); it != _components.rend(); ++it) {
            (*it)->shutdown();
        }
        _scene = nullptr;
        _app = nullptr;
    }

    glm::mat4 Camera::get_view_matrix() const {
        if (_matrices_dirty) {
            update_matrices();
        }
        return _view_matrix;
    }

    glm::mat4 Camera::get_projection_matrix() const {
        if (_matrices_dirty) {
            update_matrices();
        }
        return _projection_matrix;
    }

    Camera &Camera::set_perspective(float fov_degrees, float near_clip, float far_clip) {
        _projection_type = ProjectionType::Perspective;
        _fov = fov_degrees;
        _near_clip = near_clip;
        _far_clip = far_clip;
        _matrices_dirty = true;
        return *this;
    }

    Camera &Camera::set_ortho(const glm::vec2 &size, float near_clip, float far_clip) {
        _projection_type = ProjectionType::Orthographic;
        _ortho_size = size;
        _near_clip = near_clip;
        _far_clip = far_clip;
        _matrices_dirty = true;
        return *this;
    }

    Camera &Camera::set_ortho(float width, float height, float near_clip, float far_clip) {
        return set_ortho(glm::vec2(width, height), near_clip, far_clip);
    }

    void Camera::set_viewport(const glm::vec4 &viewport) {
        _viewport = viewport;
        _matrices_dirty = true;
    }

    void Camera::set_clear_color(const glm::vec4 &color) {
        _clear_color = color;
    }

    void Camera::set_clear_flags(uint16_t flags) {
        _clear_flags = flags;
    }

    bgfx::ViewId Camera::render_reset(bgfx::ViewId view_id) {
        _view_id = view_id;
        if (_app != nullptr) {
            auto &window = _app->get_window();
            auto size = window.get_size();
            uint16_t width = size.x;
            uint16_t height = size.y;
            uint16_t vx = static_cast<uint16_t>(_viewport.x * width);
            uint16_t vy = static_cast<uint16_t>(_viewport.y * height);
            uint16_t vw = static_cast<uint16_t>(_viewport.z * width);
            uint16_t vh = static_cast<uint16_t>(_viewport.w * height);
            bgfx::setViewRect(_view_id, vx, vy, vw, vh);
            bgfx::setViewClear(_view_id, _clear_flags,
                               static_cast<uint32_t>(_clear_color.r * 255) << 24 |
                               static_cast<uint32_t>(_clear_color.g * 255) << 16 |
                               static_cast<uint32_t>(_clear_color.b * 255) << 8 |
                               static_cast<uint32_t>(_clear_color.a * 255),
                               1.0f, 0);
            const glm::mat4 &view = get_view_matrix();
            const glm::mat4 &proj = get_projection_matrix();
            bgfx::setViewTransform(_view_id, &view[0][0], &proj[0][0]);
        }
        for (const auto &component: _components) {
            _view_id = component->render_reset(_view_id);
        }
        return _view_id + 1;
    }

    void Camera::render() const {
        for (const auto &component: _components) {
            component->render();
        }
    }

    void Camera::update(float delta_time) {
        if (_update_enabled) {
            _enabled = _update_enabled.value();
            _update_enabled.reset();
        }
        for (const auto &component: _components) {
            component->update(delta_time);
        }
        update_matrices();
    }

    void Camera::add_component_impl(std::unique_ptr<ICameraComponent> &&component) {
        if (_scene && _app) {
            component->init(*this, *_scene, *_app);
        }
        _components.push_back(std::move(component));
    }

    ICameraComponent *Camera::get_component_impl(size_t type_hash) const {
        for (const auto &component: _components) {
            if (component->get_camera_component_type() == type_hash) {
                return component.get();
            }
        }
        return nullptr;
    }

    bool Camera::remove_component_impl(size_t type_hash) {
        auto it = std::find_if(_components.begin(), _components.end(),
                              [type_hash](const auto &component) {
                                  return component->get_camera_component_type() == type_hash;
                              });
        if (it != _components.end()) {
            if (_scene && _app) {
                (*it)->shutdown();
            }
            _components.erase(it);
            return true;
        }
        return false;
    }

    Camera &Camera::set_culling_filter(std::unique_ptr<ICullingFilter> &&filter) {
        _culling_filter = std::move(filter);
        return *this;
    }

    Camera &Camera::set_culling_filter_impl(std::unique_ptr<ICullingFilter> &&filter) {
        _culling_filter = std::move(filter);
        return *this;
    }

    const ICullingFilter *Camera::get_culling_filter() const {
        return _culling_filter.get();
    }

    glm::vec3 Camera::screen_to_world_point(const glm::vec2 &screen_pos, float depth) const {
        if (!_app) return glm::vec3(0.0f);
        auto &window = _app->get_window();
        auto size = window.get_size();
        float width = static_cast<float>(size.x);
        float height = static_cast<float>(size.y);
        float x = 2.0f * (screen_pos.x / width) - 1.0f;
        float y = 1.0f - 2.0f * (screen_pos.y / height);
        glm::vec4 clip_pos(x, y, depth, 1.0f);
        glm::mat4 inv_proj = glm::inverse(get_projection_matrix());
        glm::mat4 inv_view = glm::inverse(get_view_matrix());
        glm::vec4 view_pos = inv_proj * clip_pos;
        view_pos /= view_pos.w;
        glm::vec4 world_pos = inv_view * view_pos;
        return glm::vec3(world_pos);
    }

    glm::vec2 Camera::world_to_screen_point(const glm::vec3 &world_pos) const {
        if (!_app) return glm::vec2(0.0f);
        auto &window = _app->get_window();
        auto size = window.get_size();
        float width = static_cast<float>(size.x);
        float height = static_cast<float>(size.y);
        glm::vec4 clip_pos = get_projection_matrix() * get_view_matrix() * glm::vec4(world_pos, 1.0f);
        clip_pos /= clip_pos.w;
        float screen_x = (clip_pos.x + 1.0f) * 0.5f * width;
        float screen_y = (1.0f - clip_pos.y) * 0.5f * height;
        return glm::vec2(screen_x, screen_y);
    }

    glm::vec3 Camera::screen_to_viewport_point(const glm::vec2 &screen_pos) const {
        if (!_app) return glm::vec3(0.0f);
        auto &window = _app->get_window();
        auto size = window.get_size();
        float width = static_cast<float>(size.x);
        float height = static_cast<float>(size.y);
        float vx = screen_pos.x / width;
        float vy = screen_pos.y / height;
        return glm::vec3(vx, vy, 0.0f);
    }

    glm::vec2 Camera::viewport_to_screen_point(const glm::vec3 &viewport_pos) const {
        if (!_app) return glm::vec2(0.0f);
        auto &window = _app->get_window();
        auto size = window.get_size();
        float width = static_cast<float>(size.x);
        float height = static_cast<float>(size.y);
        float screen_x = viewport_pos.x * width;
        float screen_y = viewport_pos.y * height;
        return glm::vec2(screen_x, screen_y);
    }

    Ray Camera::screen_point_to_ray(const glm::vec2 &screen_pos) const {
        Ray ray;
        if (!_app) return ray;
        if (_scene && _entity != entt::null) {
            if (const Transform *transform = _scene->get_component<Transform>(_entity)) {
                ray.origin = transform->get_position();
            }
        }
        glm::vec3 near_point = screen_to_world_point(screen_pos, 0.0f);
        glm::vec3 far_point = screen_to_world_point(screen_pos, 1.0f);
        ray.direction = glm::normalize(far_point - near_point);
        return ray;
    }

    void Camera::configure_view(bgfx::ViewId view_id, const std::string &name) const {
        bgfx::setViewName(view_id, name.c_str());
        bgfx::setViewFrameBuffer(view_id, BGFX_INVALID_HANDLE);
        bgfx::setViewMode(view_id, bgfx::ViewMode::Sequential);
        bgfx::setViewClear(view_id, _clear_flags,
                           static_cast<uint32_t>(_clear_color.r * 255) << 24 |
                           static_cast<uint32_t>(_clear_color.g * 255) << 16 |
                           static_cast<uint32_t>(_clear_color.b * 255) << 8 |
                           static_cast<uint32_t>(_clear_color.a * 255),
                           1.0f, 0);
    }

    bool Camera::is_valid() const {
        return _scene && _entity != entt::null;
    }

    bool Camera::is_enabled() const {
        if (_update_enabled) {
            return _update_enabled.value();
        }
        return _enabled;
    }

    void Camera::update_matrices() const {
        if (!_matrices_dirty) return;
        if (_scene && _entity != entt::null) {
            Transform *transform = _scene->get_component<Transform>(_entity);
            if (transform) {
                _view_matrix = glm::inverse(transform->get_model_matrix());
            } else {
                _view_matrix = glm::mat4(1.0f);
            }
        } else {
            _view_matrix = glm::mat4(1.0f);
        }
        if (_projection_type == ProjectionType::Perspective) {
            float aspect = 1.0f;
            if (_app) {
                auto &window = _app->get_window();
                auto size = window.get_size();
                float width = static_cast<float>(size.x) * _viewport.z;
                float height = static_cast<float>(size.y) * _viewport.w;
                if (height > 0.0f) {
                    aspect = width / height;
                }
            }
            _projection_matrix = glm::perspective(
                glm::radians(_fov),
                aspect,
                _near_clip,
                _far_clip
            );
        } else {
            float half_width = _ortho_size.x * 0.5f;
            float half_height = _ortho_size.y * 0.5f;
            _projection_matrix = glm::ortho(
                -half_width, half_width,
                -half_height, half_height,
                _near_clip, _far_clip
            );
        }
        _matrices_dirty = false;
    }

    bool Culling2D::is_visible(const glm::vec3 &position, float radius) const {
        return true;
    }

    bool Culling3D::is_visible(const glm::vec3 &position, float radius) const {
        return true;
    }
}
