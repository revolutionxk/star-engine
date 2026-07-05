#pragma once

namespace star::resources {
    class ResourceManager;
    
    class ResourceResolver {
      public:
        static void bind(ResourceManager* manager) {
            s_manager = manager;
        }

        static void unbind(const ResourceManager* manager) {
            if (s_manager == manager)
                s_manager = nullptr;
        }

        [[nodiscard]] static ResourceManager* active() {
            return s_manager;
        }

      private:
        static ResourceManager* s_manager;
    };
} // namespace star::resources
