#pragma once

#include "star/core/common.hpp"

namespace star::application {
    class STAR_EXPORT ApplicationContext {
      public:
        ApplicationContext() = default;
        ~ApplicationContext() = default;

        ApplicationContext(const ApplicationContext&) = delete;
        ApplicationContext& operator=(const ApplicationContext&) = delete;

        template<typename T>
        void register_service(std::unique_ptr<T> service) {
            const auto type_id = std::type_index(typeid(T));

            auto wrapper = std::make_unique<ServiceWrapper<T>>(std::move(service));
            m_services[type_id] = std::move(wrapper);
        }

        template<typename T>
        void register_service(T* service) {
            const auto type_id = std::type_index(typeid(T));

            auto wrapper = std::make_unique<ServiceWrapper<T>>(service);
            m_services[type_id] = std::move(wrapper);
        }

        template<typename T>
        [[nodiscard]] T& get_service() const {
            const auto type_id = std::type_index(typeid(T));

            const auto it = m_services.find(type_id);
            if (it == m_services.end()) {
                throw std::runtime_error("Service not registered");
            }

            auto* wrapper = static_cast<ServiceWrapper<T>*>(it->second.get());
            return wrapper->get();
        }

        template<typename T>
        [[nodiscard]] bool has_service() const {
            const auto type_id = std::type_index(typeid(T));
            return m_services.contains(type_id);
        }

        template<typename T>
        [[nodiscard]] T* try_get_service() const {
            const auto type_id = std::type_index(typeid(T));

            const auto it = m_services.find(type_id);
            if (it == m_services.end()) {
                return nullptr;
            }

            auto* wrapper = static_cast<ServiceWrapper<T>*>(it->second.get());
            return &wrapper->get();
        }

        template<typename T>
        void unregister_service() {
            const auto type_id = std::type_index(typeid(T));
            m_services.erase(type_id);
        }

        void clear() {
            m_services.clear();
        }

      private:
        struct IServiceWrapper {
            virtual ~IServiceWrapper() = default;
        };

        template<typename T>
        struct ServiceWrapper : IServiceWrapper {
            explicit ServiceWrapper(std::unique_ptr<T> owned_service)
                : m_owned_service(std::move(owned_service)), m_service(m_owned_service.get()) {}

            explicit ServiceWrapper(T* non_owned_service) : m_owned_service(nullptr), m_service(non_owned_service) {}

            T& get() {
                return *m_service;
            }

            std::unique_ptr<T> m_owned_service;
            T* m_service;
        };

        std::unordered_map<std::type_index, std::unique_ptr<IServiceWrapper>> m_services;
    };
} // namespace star::application
