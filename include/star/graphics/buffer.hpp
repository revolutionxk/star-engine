#pragma once

namespace star::graphics {
    struct BufferDescriptor {
        enum class Usage {
            Static,
            Dynamic,
            Stream
        };

        enum class BindFlags {
            VertexBuffer = 1 << 0,
            IndexBuffer = 1 << 1,
            ConstantBuffer = 1 << 2,
            ShaderResource = 1 << 3,
            UnorderedAccess = 1 << 4,
            TransferSrc = 1 << 5,
            TransferDst = 1 << 6,
        };

        enum class Type {
            Vertex,
            Index16,
            Index32,
            Constant,
            Storage
        };

        size_t size_in_bytes{0};
        Usage usage{Usage::Static};
        u32 bind_flags{0};
        Type type{Type::Vertex};
        const void* initial_data{nullptr};

        u32 stride{0};
    };

    struct Buffer;
} // namespace star::graphics
