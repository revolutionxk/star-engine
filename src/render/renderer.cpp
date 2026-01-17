#include "star/render/renderer.hpp"
#include "star/scene/scene.hpp"
#include "star/app/app.hpp"
#include <spdlog/spdlog.h>

namespace star {
    Renderer::Renderer() = default;

    Renderer::~Renderer() = default;


    void Renderer::init(Scene &scene, App &app) {
        _scene = &scene;
        _app = &app;
        if (_pipeline) {
            _pipeline->init(scene, app);
        }
        spdlog::debug("Initialized renderer: {}", get_renderer_name());
    }

    void Renderer::shutdown() {
        spdlog::debug("Shutting down renderer: {}", get_renderer_name());
        if (_pipeline) {
            _pipeline->shutdown();
            _pipeline.reset();
        }
        _camera.reset();
        _scene.reset();
        _app.reset();
    }

    void Renderer::update(float delta_time) {
        if (_pipeline) {
            _pipeline->update(delta_time);
        }
    }

    bgfx::ViewId Renderer::render_reset(bgfx::ViewId view_id) {
        if (!_visible) {
            return view_id;
        }
        _view_id = view_id;

        return view_id + 1;
    }

    void Renderer::render(bgfx::ViewId view_id, bgfx::Encoder *encoder) {
        if (!_visible || !_scene) {
            return;
        }
        if (_pipeline) {
            _pipeline->render(view_id, encoder);
        }
    }

    void Renderer::set_visible(bool visible) {
        _visible = visible;
    }

    bool Renderer::is_visible() const {
        return _visible;
    }

    void Renderer::set_debug_enabled(bool enabled) {
        _debug_enabled = enabled;
    }

    bool Renderer::is_debug_enabled() const {
        return _debug_enabled;
    }

    void Renderer::set_camera(Camera &camera) {
        _camera = camera;
    }

    OptionalRef<Camera> Renderer::get_camera() const {
        return _camera;
    }
}
