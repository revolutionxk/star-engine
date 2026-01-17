#include "star/render/render_pipeline.hpp"

namespace star
{
    void RenderPipeline::add_pass(std::unique_ptr<IRenderPass> pass)
    {
        passes_.emplace_back(std::move(pass));
    }

    void RenderPipeline::init(Scene& scene, App& app) const
    {
        for (const auto& pass : passes_)
        {
            pass->init(scene, app);
        }
    }

    void RenderPipeline::shutdown() const
    {
        for (const auto& pass : passes_)
        {
            pass->shutdown();
        }
    }

    void RenderPipeline::update(float delta_time) const
    {
        for (const auto& pass : passes_)
        {
            pass->update(delta_time);
        }
    }

    void RenderPipeline::render(bgfx::ViewId view_id, bgfx::Encoder* encoder) const
    {
        for (const auto& pass : passes_)
        {
            pass->render(view_id, encoder);
        }
    }
}
