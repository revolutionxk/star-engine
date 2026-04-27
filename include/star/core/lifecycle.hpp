#pragma once
#include "types.hpp"

namespace star {
    class IInitializable {
      public:
        virtual ~IInitializable() = default;

        virtual bool initialize() = 0;
        virtual void shutdown() = 0;
    };

    class IUpdatable {
      public:
        virtual ~IUpdatable() = default;

        virtual void update(f32 delta_time) = 0;
    };

    class IPreRenderable {
      public:
        virtual ~IPreRenderable() = default;

        virtual void pre_render(f32 delta_time) = 0;
    };

    class IRenderable {
      public:
        virtual ~IRenderable() = default;

        virtual void render() = 0;
    };

    class IImGuiRenderable {
      public:
        virtual ~IImGuiRenderable() = default;

        virtual void on_imgui_render() = 0;
    };
} // namespace star
