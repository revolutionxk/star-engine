#include "star/graphics/command_buffer.hpp"

#include "star/core/common.hpp"

namespace star::graphics {

    CommandBuffer::CommandBuffer() {
        m_commands.reserve(128);
    }

    CommandBuffer::~CommandBuffer() {
        clear();
    }

    CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
        : m_commands(std::move(other.m_commands)), m_is_recording(other.m_is_recording) {
        other.m_is_recording = false;
    }

    CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept {
        if (this != &other) {
            m_commands = std::move(other.m_commands);
            m_is_recording = other.m_is_recording;
            other.m_is_recording = false;
        }
        return *this;
    }

    void CommandBuffer::begin() {
        STAR_ASSERT(!m_is_recording, "CommandBuffer::begin() called while already recording");
        clear();
        m_is_recording = true;
    }

    void CommandBuffer::end() {
        STAR_ASSERT(m_is_recording, "CommandBuffer::end() called without begin()");
        m_is_recording = false;

        STAR_LOG_TRACE(LogCategory::Rendering, "CommandBuffer recorded {} commands", m_commands.size());
    }

    void CommandBuffer::submit(Device& device) const {
        STAR_ASSERT(!m_is_recording, "CommandBuffer::submit() called while still recording");

        STAR_LOG_TRACE(LogCategory::Rendering, "Executing {} commands", m_commands.size());

        for (auto& command : m_commands) {
            if (command) {
                command->execute(device);
            }
        }
    }

    void CommandBuffer::clear() {
        m_commands.clear();
    }

    void SetViewportCommand::execute(Device& device) {
        // device.set_viewport(m_x, m_y, m_width, m_height);
    }

    void SetScissorCommand::execute(Device& device) {
        // device.set_scissor(m_x, m_y, m_width, m_height);
    }

    void ClearColorCommand::execute(Device& device) {
        // device.clear_color(m_r, m_g, m_b, m_a);
    }

} // namespace star::graphics
