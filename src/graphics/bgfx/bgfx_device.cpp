#include "bgfx_device.hpp"

#include <bgfx/bgfx.h>

#include "bgfx_device_context.hpp"
#include "bgfx_vertex_layout.hpp"
#include "star/core/common.hpp"
#include "star/graphics/buffer.hpp"
#include "star/graphics/shader.hpp"
#include "star/graphics/texture.hpp"
#include "star/platform/window.hpp"

static bgfx::TextureFormat::Enum to_bgfx_format(const graphics::TextureDescriptor::Format format) {
    using Format = graphics::TextureDescriptor::Format;
    switch (format) {
        case Format::RGBA8:
            return bgfx::TextureFormat::RGBA8;
        case Format::RGB8:
            return bgfx::TextureFormat::RGB8;
        case Format::BGRA8:
            return bgfx::TextureFormat::BGRA8;
        case Format::RGBA16F:
            return bgfx::TextureFormat::RGBA16F;
        case Format::RGBA32F:
            return bgfx::TextureFormat::RGBA32F;
        case Format::Depth24Stencil8:
            return bgfx::TextureFormat::D24S8;
        default:
            return bgfx::TextureFormat::RGBA8;
    }
}

namespace star::graphics {
    BGFXDevice::~BGFXDevice() {
        if (!m_initialized) {
            return;
        }

        shutdown();
    }

    std::unique_ptr<DeviceContext> BGFXDevice::create_context() {
        return std::make_unique<BGFXDeviceContext>(m_config.platform.native_window_handle);
    }

    bool BGFXDevice::initialize(GraphicsDeviceConfig& device) {
        bgfx::Init init;
        bgfx::PlatformData platform_data{};

        if (device.platform.native_window_handle == nullptr) {
            STAR_LOG_ERROR(LogCategory::Graphics, "No window available for BGFX initialization");
            return false;
        }

        platform_data.nwh = device.platform.native_window_handle;
        platform_data.ndt = device.platform.native_display_type;

        init.platformData = platform_data;
        init.type = bgfx::RendererType::Count;

        const auto size = device.window_size;

        init.resolution.width = static_cast<u32>(size.x);
        init.resolution.height = static_cast<u32>(size.y);
        init.debug = false;
        init.resolution.reset = 0;

        if (!bgfx::init(init)) {
            STAR_LOG_ERROR(LogCategory::Graphics, "Failed to initialize bgfx");
            return false;
        }

        constexpr auto clear_hex = static_cast<uint32_t>(0x443355FF);

        bgfx::setPaletteColor(0, 0xFF000000);
        bgfx::setPaletteColor(1, clear_hex);
        bgfx::setPaletteColor(2, 0xFFFFFFFF);

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clear_hex, 1.0f, 0);

        bgfx::setViewRect(0, 0, 0, init.resolution.width, init.resolution.height);

        m_initialized = true;

        const auto caps = bgfx::getCaps();
        STAR_LOG_INFO(LogCategory::Graphics, "Graphics API: {}", bgfx::getRendererName(caps->rendererType));

        return true;
    }

    void BGFXDevice::shutdown() {
        bgfx::shutdown();

        m_initialized = false;
    }

    DeviceCaps BGFXDevice::caps() const {
        const auto caps = bgfx::getCaps();

        DeviceCaps device_caps{
            .renderer_name = bgfx::getRendererName(caps->rendererType),
            .vendor_name = caps->vendorId == BGFX_PCI_ID_NONE ? "Unknown" : std::to_string(caps->vendorId),
            .max_texture_size = static_cast<int>(caps->limits.maxTextureSize),
            .max_texture_units = static_cast<int>(caps->limits.maxTextureSamplers),
            .supports_compute_shaders = (caps->supported & BGFX_CAPS_COMPUTE) != 0,
        };

        return device_caps;
    }

    ResourceHandle<Buffer> BGFXDevice::create_buffer(BufferDescriptor& buffer_descriptor) {
        const bgfx::Memory* mem = nullptr;
        if (buffer_descriptor.initial_data != nullptr) {
            mem = bgfx::copy(buffer_descriptor.initial_data, buffer_descriptor.size_in_bytes);
        }

        u32 handle_id = 0;
        const auto is_dynamic = buffer_descriptor.usage != BufferDescriptor::Usage::Static;

        switch (buffer_descriptor.type) {
            case BufferDescriptor::Type::Vertex: {
                const auto layout = create_standard_vertex_layout();

                bgfx::VertexBufferHandle bgfx_handle;
                if (is_dynamic) {
                    const auto num_vertices = static_cast<u32>(buffer_descriptor.size_in_bytes / sizeof(Vertex));
                    auto dynamic_handle = bgfx::createDynamicVertexBuffer(num_vertices, layout);

                    if (mem) {
                        bgfx::update(dynamic_handle, 0, mem);
                    }

                    bgfx_handle = reinterpret_cast<bgfx::VertexBufferHandle&>(dynamic_handle);
                } else {
                    if (!mem) {
                        STAR_LOG_ERROR(LogCategory::Graphics, "Static vertex buffer requires initial data");
                        return ResourceHandle<Buffer>{0, 0};
                    }
                    bgfx_handle = bgfx::createVertexBuffer(mem, layout);
                }

                if (!bgfx::isValid(bgfx_handle)) {
                    STAR_LOG_ERROR(LogCategory::Graphics, "Failed to create vertex buffer");
                    return ResourceHandle<Buffer>{0, 0};
                }

                handle_id = bgfx_handle.idx;
                break;
            }

            case BufferDescriptor::Type::Index16:
            case BufferDescriptor::Type::Index32: {
                const bool is_32bit = buffer_descriptor.type == BufferDescriptor::Type::Index32;
                const u32 flags = is_32bit ? BGFX_BUFFER_INDEX32 : 0;

                bgfx::IndexBufferHandle bgfx_handle;
                if (is_dynamic) {
                    const auto num_indices = static_cast<u32>(buffer_descriptor.size_in_bytes / (is_32bit ? 4 : 2));
                    auto dynamic_handle = bgfx::createDynamicIndexBuffer(num_indices, flags);
                    if (mem) {
                        bgfx::update(dynamic_handle, 0, mem);
                    }
                    bgfx_handle = reinterpret_cast<bgfx::IndexBufferHandle&>(dynamic_handle);
                } else {
                    if (!mem) {
                        STAR_LOG_ERROR(LogCategory::Graphics, "Static index buffer requires initial data");
                        return ResourceHandle<Buffer>{0, 0};
                    }
                    bgfx_handle = bgfx::createIndexBuffer(mem, flags);
                }

                if (!bgfx::isValid(bgfx_handle)) {
                    STAR_LOG_ERROR(LogCategory::Graphics, "Failed to create index buffer");
                    return ResourceHandle<Buffer>{0, 0};
                }

                handle_id = bgfx_handle.idx;
                break;
            }

            default:
                STAR_LOG_ERROR(LogCategory::Graphics, "Unsupported buffer type");
                return ResourceHandle<Buffer>{0, 0};
        }

        STAR_LOG_DEBUG(LogCategory::Graphics, "Created buffer (handle: {}, size: {} bytes)", handle_id,
                       buffer_descriptor.size_in_bytes);

        return ResourceHandle<Buffer>{handle_id, 0};
    }

    ResourceHandle<Texture> BGFXDevice::create_texture(TextureDescriptor& texture_descriptor) {
        const bgfx::Memory* mem = nullptr;
        if (texture_descriptor.initial_data != nullptr) {
            mem = bgfx::copy(texture_descriptor.initial_data, texture_descriptor.size_in_bytes);
        }

        bgfx::TextureFormat::Enum bgfx_format = to_bgfx_format(texture_descriptor.format);

        bgfx::TextureHandle bgfx_handle =
            bgfx::createTexture2D(static_cast<u16>(texture_descriptor.width),
                                  static_cast<u16>(texture_descriptor.height), texture_descriptor.mip_levels > 1,
                                  1, // Layers
                                  bgfx_format, BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, mem);

        if (!bgfx::isValid(bgfx_handle)) {
            STAR_LOG_ERROR(LogCategory::Graphics, "Failed to create texture ({}x{})", texture_descriptor.width,
                           texture_descriptor.height);
            return ResourceHandle<Texture>{0, 0};
        }

        return ResourceHandle<Texture>{bgfx_handle.idx, 0};
    }

    ResourceHandle<Shader> BGFXDevice::create_shader(ShaderDescriptor& shader_descriptor) {
        if (shader_descriptor.stages.empty()) {
            STAR_LOG_ERROR(LogCategory::Graphics, "Shader descriptor has no stages");
            return ResourceHandle<Shader>{0, 0};
        }

        bgfx::ShaderHandle vertex_shader = BGFX_INVALID_HANDLE;
        bgfx::ShaderHandle fragment_shader = BGFX_INVALID_HANDLE;
        bgfx::ShaderHandle compute_shader = BGFX_INVALID_HANDLE;

        for (const auto& stage : shader_descriptor.stages) {
            if (stage.bytecode.empty()) {
                STAR_LOG_ERROR(LogCategory::Graphics, "Shader stage has no bytecode");
                continue;
            }

            const bgfx::Memory* mem = bgfx::copy(stage.bytecode.data(), static_cast<u32>(stage.bytecode.size()));
            bgfx::ShaderHandle shader_handle = bgfx::createShader(mem);

            if (!bgfx::isValid(shader_handle)) {
                STAR_LOG_ERROR(LogCategory::Graphics, "Failed to create shader stage");

                if (bgfx::isValid(vertex_shader))
                    bgfx::destroy(vertex_shader);
                if (bgfx::isValid(fragment_shader))
                    bgfx::destroy(fragment_shader);
                if (bgfx::isValid(compute_shader))
                    bgfx::destroy(compute_shader);

                return ResourceHandle<Shader>{0, 0};
            }

            switch (stage.stage) {
                case ShaderDescriptor::Stage::Vertex:
                    vertex_shader = shader_handle;
                    break;
                case ShaderDescriptor::Stage::Fragment:
                    fragment_shader = shader_handle;
                    break;
                case ShaderDescriptor::Stage::Compute:
                    compute_shader = shader_handle;
                    break;
            }
        }

        bgfx::ProgramHandle program;

        if (bgfx::isValid(compute_shader)) {
            program = bgfx::createProgram(compute_shader, true);
        } else if (bgfx::isValid(vertex_shader) && bgfx::isValid(fragment_shader)) {
            program = bgfx::createProgram(vertex_shader, fragment_shader, true);
        } else {
            STAR_LOG_ERROR(LogCategory::Graphics, "Invalid shader stage combination");

            if (bgfx::isValid(vertex_shader))
                bgfx::destroy(vertex_shader);
            if (bgfx::isValid(fragment_shader))
                bgfx::destroy(fragment_shader);
            if (bgfx::isValid(compute_shader))
                bgfx::destroy(compute_shader);

            return ResourceHandle<Shader>{0, 0};
        }

        if (!bgfx::isValid(program)) {
            STAR_LOG_ERROR(LogCategory::Graphics, "Failed to create shader program");
            return ResourceHandle<Shader>{0, 0};
        }

        STAR_LOG_DEBUG(LogCategory::Graphics, "Created shader program '{}' (handle: {})", shader_descriptor.name,
                       program.idx);

        return ResourceHandle<Shader>{program.idx, 0};
    }

    void BGFXDevice::destroy_buffer(ResourceHandle<Buffer> handle) {
        if (!handle.is_valid()) {
            return;
        }

        const bgfx::VertexBufferHandle vb_handle{static_cast<u16>(handle.id)};
        const bgfx::IndexBufferHandle ib_handle{static_cast<u16>(handle.id)};
        const bgfx::DynamicVertexBufferHandle dvb_handle{static_cast<u16>(handle.id)};
        const bgfx::DynamicIndexBufferHandle dib_handle{static_cast<u16>(handle.id)};

        if (bgfx::isValid(vb_handle)) {
            bgfx::destroy(vb_handle);
        } else if (bgfx::isValid(ib_handle)) {
            bgfx::destroy(ib_handle);
        } else if (bgfx::isValid(dvb_handle)) {
            bgfx::destroy(dvb_handle);
        } else if (bgfx::isValid(dib_handle)) {
            bgfx::destroy(dib_handle);
        }

        STAR_LOG_DEBUG(LogCategory::Graphics, "Destroyed buffer (handle: {})", handle.id);
    }

    void BGFXDevice::destroy_texture(const ResourceHandle<Texture> handle) {
        if (!handle.is_valid()) {
            return;
        }

        bgfx::TextureHandle bgfx_handle{static_cast<u16>(handle.id)};

        if (bgfx::isValid(bgfx_handle)) {
            bgfx::destroy(bgfx_handle);
        }
    }

    void BGFXDevice::destroy_shader(const ResourceHandle<Shader> handle) {
        if (!handle.is_valid()) {
            return;
        }

        bgfx::ProgramHandle bgfx_handle{static_cast<u16>(handle.id)};

        if (bgfx::isValid(bgfx_handle)) {
            bgfx::destroy(bgfx_handle);
        }
    }
} // namespace star::graphics
