#include "star/scene/scene.hpp"
#include "star/scene/entity_registry.hpp"
#include "star/app/app.hpp"
#include <algorithm>
#include <spdlog/spdlog.h>

#include "star/scene/camera.hpp"
#include "star/scene/transform.hpp"
#include "star/render/renderer_components.hpp"

namespace star {
    SceneImpl::SceneImpl(Scene &scene)
        : _scene(scene)
          , _name("Scene") {
    }

    SceneImpl::~SceneImpl() {
        shutdown();
    }

    void SceneImpl::init(App &app) {
        _app = &app;

        for (auto &component: _components) {
            component->init(_scene, app);
        }

        auto &cams = _registry.storage<Camera>();
        for (auto itr = cams.rbegin(), last = cams.rend(); itr != last; ++itr) {
            itr->get_impl()->init(_scene, app);
        }

        _registry.on_construct<Camera>().connect<&SceneImpl::on_camera_constructed>(*this);
        _registry.on_destroy<Camera>().connect<&SceneImpl::on_camera_destroyed>(*this);
    }

    void SceneImpl::on_camera_constructed(EntityRegistry &registry, const Entity entity) const {
        if (_app) {
            const auto &cam = registry.get<Camera>(entity);
            cam.get_impl()->init(_scene, *_app);
        }
    }

    void SceneImpl::on_camera_destroyed(EntityRegistry &registry, const Entity entity) const {
        if (_app) {
            const auto &cam = registry.get<Camera>(entity);
            cam.get_impl()->shutdown();
        }
    }

    void SceneImpl::shutdown() {
        if (!_app) {
            return;
        }

        for (auto it = _components.rbegin(); it != _components.rend(); ++it) {
            (*it)->shutdown();
        }

        _app = nullptr;
    }

    void SceneImpl::render() {
        auto &cams = _registry.storage<Camera>();
        for (auto itr = cams.rbegin(), last = cams.rend(); itr != last; ++itr) {
            itr->get_impl()->render();
        }
    }

    void SceneImpl::update(const float delta_time) const {
        if (_paused) {
            return;
        }

        for (const auto &camera: _registry.view<Camera>()) {
            auto &cam = _registry.get<Camera>(camera);
            cam.get_impl()->update(delta_time);
        }

        for (const auto &component: _components) {
            component->update(delta_time);
        }

        if (_delegate) {
            _delegate->on_scene_updated(delta_time);
        }
    }

    bgfx::ViewId SceneImpl::render_reset(bgfx::ViewId view_id) {
        _view_id = view_id;

        for (auto &component: _components) {
            _view_id = component->render_reset(_view_id);
        }

        return _view_id;
    }

    void SceneImpl::set_paused(bool paused) {
        _paused = paused;
    }

    bool SceneImpl::is_paused() const {
        return _paused;
    }

    void SceneImpl::set_name(const std::string &name) {
        _name = name;
    }

    void SceneImpl::clear() {
        _registry.clear();
    }

    const std::string &SceneImpl::get_name() const {
        return _name;
    }

    void SceneImpl::add_scene_component(std::unique_ptr<ISceneComponent> &&component) {
        if (const auto type_hash = component->get_scene_component_type()) {
            remove_scene_component(type_hash);
        }

        if (_app) {
            component->init(_scene, *_app);
        }

        _components.push_back(std::move(component));
    }

    ISceneComponent *SceneImpl::get_scene_component(size_t type_hash) {
        for (auto &component: _components) {
            if (component->get_scene_component_type() == type_hash) {
                return component.get();
            }
        }
        return nullptr;
    }

    bool SceneImpl::remove_scene_component(size_t type_hash) {
        auto it = std::find_if(_components.begin(), _components.end(),
                               [type_hash](const auto &component) {
                                   return component->get_scene_component_type() == type_hash;
                               });

        if (it != _components.end()) {
            if (_app) {
                (*it)->shutdown();
            }
            _components.erase(it);
            return true;
        }

        return false;
    }

    Entity SceneImpl::create_entity() {
        const Entity entity = _registry.create();

        if (_delegate) {
            _delegate->on_entity_created(entity);
        }

        return entity;
    }

    void SceneImpl::destroy_entity(Entity entity) {
        if (_registry.valid(entity)) {
            if (_delegate) {
                _delegate->on_entity_destroyed(entity);
            }

            _registry.destroy(entity);
        }
    }

    bool SceneImpl::is_valid_entity(Entity entity) const {
        return _registry.valid(entity);
    }

    void SceneImpl::set_delegate(ISceneDelegate *delegate) {
        _delegate = delegate;
    }

    ISceneDelegate *SceneImpl::get_delegate() const {
        return _delegate;
    }

    EntityRegistry &SceneImpl::get_registry() {
        return _registry;
    }

    const EntityRegistry &SceneImpl::get_registry() const {
        return _registry;
    }

    void SceneImpl::set_view_id(bgfx::ViewId view_id) {
        _view_id = view_id;
    }

    bgfx::ViewId SceneImpl::get_view_id() const {
        return _view_id;
    }

    std::string SceneImpl::to_string() const {
        return "Scene(" + _name + ")";
    }

    Scene::Scene() : _impl(std::make_unique<SceneImpl>(*this)) {
    }

    Scene::~Scene() = default;

    void Scene::init(App &app) const {
        _impl->init(app);
    }

    void Scene::shutdown() {
        _impl->shutdown();
    }

    void Scene::render() const {
        _impl->render();
    }

    void Scene::update(const float delta_time) const {
        _impl->update(delta_time);
    }

    bgfx::ViewId Scene::render_reset(bgfx::ViewId view_id) {
        return _impl->render_reset(view_id);
    }

    void Scene::set_paused(bool paused) const {
        _impl->set_paused(paused);
    }

    bool Scene::is_paused() const {
        return _impl->is_paused();
    }

    void Scene::set_name(const std::string &name) {
        _impl->set_name(name);
    }

    const std::string &Scene::get_name() const {
        return _impl->get_name();
    }

    void Scene::add_scene_component_impl(std::unique_ptr<ISceneComponent> &&component) {
        _impl->add_scene_component(std::move(component));
    }

    ISceneComponent *Scene::get_scene_component_impl(size_t type_hash) {
        return _impl->get_scene_component(type_hash);
    }

    bool Scene::remove_scene_component_impl(size_t type_hash) {
        return _impl->remove_scene_component(type_hash);
    }

    Entity Scene::create_entity() {
        return _impl->create_entity();
    }

    void Scene::destroy_entity(const Entity entity) {
        _impl->destroy_entity(entity);
    }

    bool Scene::is_valid_entity(const Entity entity) const {
        return _impl->is_valid_entity(entity);
    }

    void Scene::set_delegate(ISceneDelegate *delegate) {
        _impl->set_delegate(delegate);
    }

    void Scene::clear() {
        _impl->clear();
    }

    EntityRegistry &Scene::get_registry() {
        return _impl->get_registry();
    }

    ISceneDelegate *Scene::get_delegate() const {
        return _impl->get_delegate();
    }

    SceneAppComponent::SceneAppComponent()
        : _scene(std::make_unique<Scene>()) {
    }

    SceneAppComponent::~SceneAppComponent() = default;

    void SceneAppComponent::init(App &app) {
        _app = &app;
        _scene->init(app);
        _scene->set_delegate(this);
    }

    void SceneAppComponent::render() {
        _scene->render();
    }

    void SceneAppComponent::update(float delta_time) {
        if (_auto_update) {
            _scene->update(delta_time);
        }
    }

    void SceneAppComponent::shutdown() {
        if (_scene) {
            _scene->set_delegate(nullptr);
            _scene->shutdown();
        }
        _app = nullptr;
    }

    Scene *SceneAppComponent::get_scene() {
        return _scene.get();
    }

    const Scene *SceneAppComponent::get_scene() const {
        return _scene.get();
    }

    void SceneAppComponent::set_auto_update(bool enabled) {
        _auto_update = enabled;
    }

    bool SceneAppComponent::get_auto_update() const {
        return _auto_update;
    }

    void SceneAppComponent::set_auto_render_reset(bool enabled) {
        _auto_render_reset = enabled;
    }

    bool SceneAppComponent::get_auto_render_reset() const {
        return _auto_render_reset;
    }
}
