#pragma once

#include <bgfx/bgfx.h>
#include <string>
#include <vector>
#include <memory>

namespace star {
    class Scene;
    class App;
    class IRenderPass {
    public:
        virtual ~IRenderPass() = default;
        virtual void init(Scene& scene, App& app) {}
        virtual void shutdown() {}
        virtual void update(float delta_time) {}
        virtual void render(bgfx::ViewId view_id, bgfx::Encoder* encoder) = 0;
        virtual std::string get_name() const = 0;
    };

    class RenderPipeline {
    public:
        void add_pass(std::unique_ptr<IRenderPass> pass);
        void init(Scene& scene, App& app) const;
        void shutdown() const;
        void update(float delta_time) const;
        void render(bgfx::ViewId view_id, bgfx::Encoder* encoder) const;
        std::vector<std::unique_ptr<IRenderPass>>& get_passes() { return passes_; }
    private:
        std::vector<std::unique_ptr<IRenderPass>> passes_;
    };
}
