#pragma once
#include <string>
#include <string_view>

#include "star/core/lifecycle.hpp"
#include "star/core/types.hpp"

namespace star::application {
    class STAR_EXPORT Layer : public IInitializable, public IUpdatable, public IRenderable, public IImGuiRenderable {
      public:
        explicit Layer(std::string_view name = "Layer");
        virtual ~Layer() = default;

        bool initialize() override {
            return true;
        }

        void shutdown() override {}

        void update(f32 delta_time) override {}

        void render() override {}

        void on_imgui_render() override {}

        virtual void on_attach() {}

        virtual void on_detach() {}

        // TODO: Add event handling
        // virtual void on_event(Event& event) {}

        [[nodiscard]] const std::string& name() const {
            return m_name;
        }

      protected:
        std::string m_name;
    };
} // namespace star::application
