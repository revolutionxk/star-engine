#pragma once

#include "star/core/types.hpp"

namespace star::graphics {
    class Device;
    class DeviceContext;
} // namespace star::graphics

namespace star::resources {
    class ResourceManager;
}

namespace star::rendering {
    class Viewport;
    struct RenderScene;

    struct FrameContext {
        graphics::Device& device;
        resources::ResourceManager& resources;

        const RenderScene* scene{nullptr};

        Viewport* viewport{nullptr};

        f32 delta_time{0.0f};
        u64 frame_index{0};
    };

    struct RenderContext {
        const FrameContext& frame;
        graphics::DeviceContext& gpu;
        u32 view_id{0};
    };
} // namespace star::rendering
