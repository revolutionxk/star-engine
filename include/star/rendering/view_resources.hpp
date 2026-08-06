#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>

#include "star/core/types.hpp"
#include "star/graphics/texture.hpp"
#include "star/rendering/render_target.hpp"

namespace star::graphics {
    class Device;
} // namespace star::graphics

namespace star::rendering {
    class ViewResources {
      public:
        explicit ViewResources(graphics::Device& device) : m_device(&device) {}
        ~ViewResources() = default;

        ViewResources(const ViewResources&) = delete;
        ViewResources& operator=(const ViewResources&) = delete;

        RenderTarget* target(std::string_view id, u32 width, u32 height, graphics::TextureFormat format,
                             bool with_depth = false);

        [[nodiscard]] RenderTarget* find(std::string_view id) const;

        u64& counter(std::string_view id, u64 initial = 0);

        void clear();

        [[nodiscard]] std::size_t target_count() const noexcept {
            return m_targets.size();
        }

      private:
        struct Entry {
            std::unique_ptr<RenderTarget> target;
            u32 width{0};
            u32 height{0};
            graphics::TextureFormat format{graphics::TextureFormat::RGBA8};
            bool depth{false};
        };

        graphics::Device* m_device;
        std::map<std::string, Entry, std::less<>> m_targets;
        std::map<std::string, u64, std::less<>> m_counters;
    };
} // namespace star::rendering
