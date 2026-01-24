#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "star/core/types.hpp"

namespace star::graphics {
    class Device;

    class STAR_EXPORT RenderCommand {
      public:
        virtual ~RenderCommand() = default;

        virtual void execute(Device& device) = 0;

        [[nodiscard]] virtual const char* name() const = 0;
    };

    class STAR_EXPORT CommandBuffer {
      public:
        CommandBuffer();
        ~CommandBuffer();

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;

        CommandBuffer(CommandBuffer&&) noexcept;
        CommandBuffer& operator=(CommandBuffer&&) noexcept;

        void begin();
        void end();

        template<typename T, typename... Args>
        void record(Args&&... args) {
            static_assert(std::is_base_of_v<RenderCommand, T>, "T must derive from RenderCommand");
            m_commands.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        }

        void submit(Device& device) const;
        void clear();

        [[nodiscard]] size_t command_count() const {
            return m_commands.size();
        }

        [[nodiscard]] bool is_recording() const {
            return m_is_recording;
        }

      private:
        std::vector<std::unique_ptr<RenderCommand>> m_commands;
        bool m_is_recording{false};
    };

    class STAR_EXPORT SetViewportCommand : public RenderCommand {
      public:
        SetViewportCommand(u32 x, u32 y, u32 width, u32 height) : m_x(x), m_y(y), m_width(width), m_height(height) {}

        void execute(Device& device) override;

        [[nodiscard]] const char* name() const override {
            return "SetViewport";
        }

      private:
        u32 m_x, m_y, m_width, m_height;
    };

    class STAR_EXPORT SetScissorCommand : public RenderCommand {
      public:
        SetScissorCommand(u32 x, u32 y, u32 width, u32 height) : m_x(x), m_y(y), m_width(width), m_height(height) {}

        void execute(Device& device) override;

        [[nodiscard]] const char* name() const override {
            return "SetScissor";
        }

      private:
        u32 m_x, m_y, m_width, m_height;
    };

    class STAR_EXPORT ClearColorCommand : public RenderCommand {
      public:
        ClearColorCommand(f32 r, f32 g, f32 b, f32 a = 1.0f) : m_r(r), m_g(g), m_b(b), m_a(a) {}

        void execute(Device& device) override;

        [[nodiscard]] const char* name() const override {
            return "ClearColor";
        }

      private:
        f32 m_r, m_g, m_b, m_a;
    };

    class STAR_EXPORT LambdaCommand : public RenderCommand {
      public:
        using CommandFunc = std::function<void(Device&)>;

        explicit LambdaCommand(CommandFunc func, const char* debug_name = "Lambda")
            : m_func(std::move(func)), m_name(debug_name) {}

        void execute(Device& device) override {
            if (m_func) {
                m_func(device);
            }
        }

        [[nodiscard]] const char* name() const override {
            return m_name;
        }

      private:
        CommandFunc m_func;
        const char* m_name;
    };

} // namespace star::graphics
