#pragma once
#include <memory>
#include <vector>

#include "layer.hpp"

namespace star::application {
    class STAR_EXPORT LayerStack {
      public:
        LayerStack() = default;
        ~LayerStack();

        void push_layer(std::unique_ptr<Layer> layer);
        void push_overlay(std::unique_ptr<Layer> overlay);
        void pop_layer(Layer* layer);
        void pop_overlay(Layer* overlay);

        auto begin() {
            return m_layers.begin();
        }

        auto end() {
            return m_layers.end();
        }

        auto rbegin() {
            return m_layers.rbegin();
        }

        auto rend() {
            return m_layers.rend();
        }

        [[nodiscard]] auto begin() const {
            return m_layers.begin();
        }

        [[nodiscard]] auto end() const {
            return m_layers.end();
        }

        [[nodiscard]] auto rbegin() const {
            return m_layers.rbegin();
        }

        [[nodiscard]] auto rend() const {
            return m_layers.rend();
        }

      private:
        std::vector<std::unique_ptr<Layer>> m_layers;
        u32 m_layer_insert_index = 0;
    };
} // namespace star::application
