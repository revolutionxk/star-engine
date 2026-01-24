#include "star/application/layer_stack.hpp"

#include <algorithm>

namespace star::application {
    LayerStack::~LayerStack() {
        for (const auto& layer : m_layers) {
            layer->on_detach();
            layer->shutdown();
        }
    }

    void LayerStack::push_layer(std::unique_ptr<Layer> layer) {
        m_layers.emplace(m_layers.begin() + m_layer_insert_index, std::move(layer));
        m_layer_insert_index++;
    }

    void LayerStack::push_overlay(std::unique_ptr<Layer> overlay) {
        m_layers.emplace_back(std::move(overlay));
    }

    void LayerStack::pop_layer(Layer* layer) {
        const auto it = std::find_if(m_layers.begin(), m_layers.begin() + m_layer_insert_index,
                                     [layer](const auto& l) { return l.get() == layer; });

        if (it != m_layers.begin() + m_layer_insert_index) {
            (*it)->on_detach();
            (*it)->shutdown();
            m_layers.erase(it);
            m_layer_insert_index--;
        }
    }

    void LayerStack::pop_overlay(Layer* overlay) {
        const auto it = std::find_if(m_layers.begin() + m_layer_insert_index, m_layers.end(),
                                     [overlay](const auto& l) { return l.get() == overlay; });

        if (it != m_layers.end()) {
            (*it)->on_detach();
            (*it)->shutdown();
            m_layers.erase(it);
        }
    }

    void LayerStack::clear() {
        for (const auto& layer : m_layers) {
            layer->on_detach();
            layer->shutdown();
        }
        m_layers.clear();
        m_layer_insert_index = 0;
    }
} // namespace star::application
