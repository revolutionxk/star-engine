#pragma once

#include "star/export.hpp"
#include "star/render/render_fwd.hpp"
#include "star/scene/camera.hpp"

#include <bgfx/bgfx.h>
#include <string>
#include "star/render/render_pipeline.hpp"

#include "star/utils/memory/optional_ref.hpp"
#include <memory>
#include <optional>

namespace star {
    class App;
    class Scene;
    class Transform;
    class Mesh;
    class Material;

    class STAR_EXPORT IRenderer {
    public:
        virtual ~IRenderer() = default;

        virtual void init(Scene &scene, App &app) {
        }

        virtual void shutdown() {
        }

        virtual void render(Scene &scene, App &app) {
        }

        virtual void update(float delta_time) {
        }

        virtual bgfx::ViewId render_reset(bgfx::ViewId view_id) { return view_id; }

        virtual void render(bgfx::ViewId view_id, bgfx::Encoder *encoder = nullptr) {
        }

        virtual RendererType get_renderer_type() const = 0;

        virtual std::string get_renderer_name() const = 0;
    };

    class STAR_EXPORT Renderer : public IRenderer {
    public:
        Renderer();

        ~Renderer() override;

        void init(Scene &scene, App &app) override;

        void shutdown() override;

        void update(float delta_time) override;

        bgfx::ViewId render_reset(bgfx::ViewId view_id) override;

        void render(bgfx::ViewId view_id, bgfx::Encoder *encoder = nullptr) override;

        void set_visible(bool visible);

        bool is_visible() const;

        void set_debug_enabled(bool enabled);

        bool is_debug_enabled() const;

        void set_camera(Camera &camera);

        OptionalRef<Camera> get_camera() const;

        virtual void on_window_resize(uint32_t width, uint32_t height) {
        }

    protected:
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
        OptionalRef<Camera> _camera;
        bool _visible{true};
        bool _debug_enabled{false};
        std::optional<bgfx::ViewId> _view_id;
        std::unique_ptr<RenderPipeline> _pipeline;
    public:
        RenderPipeline* get_pipeline() { return _pipeline.get(); }
        const RenderPipeline* get_pipeline() const { return _pipeline.get(); }
        void set_pipeline(std::unique_ptr<RenderPipeline> pipeline) { _pipeline = std::move(pipeline); }
    };
}
