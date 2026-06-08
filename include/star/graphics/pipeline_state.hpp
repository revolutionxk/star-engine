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
        Equal,
        Always,
        None
    };
    enum class PrimitiveType {
        Triangles,
        Lines,
        Points,
    };

    struct PipelineState {
        BlendMode blend_mode = BlendMode::Opaque;
        CullMode cull = CullMode::Back;
        DepthTest depth_test = DepthTest::LessEqual;
        PrimitiveType primitive = PrimitiveType::Triangles;
        bool depth_write = true;
        bool wireframe = false;
    };
} // namespace star::graphics
