#pragma once

namespace star::graphics {
    enum class BlendMode {
        Opaque,
        AlphaBlend,
        Additive,
        Premultiplied
    };
    enum class CullMode {
        None,
        Back,
        Front
    };
    enum class DepthTest {
        Less,
        LessEqual,
        Always,
        None
    };

    struct PipelineState {
        BlendMode blend_mode = BlendMode::Opaque;
        CullMode cull = CullMode::Back;
        DepthTest depth_test = DepthTest::LessEqual;
        bool depth_write = true;
        bool wireframe = false;
    };
} // namespace star::graphics
