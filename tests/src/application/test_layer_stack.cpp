#include <catch2/catch_test_macros.hpp>

#include "star/application/layer_stack.hpp"

using namespace star::application;

class TrackingLayer final : public Layer {
  public:
    explicit TrackingLayer(const std::string_view name) : Layer(name) {}

    bool attached{false};
    bool detached{false};

    void on_attach() override {
        attached = true;
    }

    void on_detach() override {
        detached = true;
    }
};

TEST_CASE("LayerStack - empty on construction", "[application][layer_stack]") {
    LayerStack stack;
    REQUIRE(stack.begin() == stack.end());
}

TEST_CASE("LayerStack - push_layer adds one entry", "[application][layer_stack]") {
    LayerStack stack;
    stack.push_layer(std::make_unique<TrackingLayer>("A"));
    REQUIRE(stack.begin() != stack.end());
}

TEST_CASE("LayerStack - layers are iterable", "[application][layer_stack]") {
    LayerStack stack;
    stack.push_layer(std::make_unique<TrackingLayer>("A"));
    stack.push_layer(std::make_unique<TrackingLayer>("B"));
    stack.push_layer(std::make_unique<TrackingLayer>("C"));

    std::vector<std::string> names;
    for (const auto& l : stack)
        names.push_back(l->name());

    REQUIRE(names.size() == 3u);
    REQUIRE(names[0] == "A");
    REQUIRE(names[1] == "B");
    REQUIRE(names[2] == "C");
}

TEST_CASE("LayerStack - overlays come after all layers", "[application][layer_stack]") {
    LayerStack stack;
    stack.push_layer(std::make_unique<TrackingLayer>("Layer"));
    stack.push_overlay(std::make_unique<TrackingLayer>("Overlay"));

    auto it = stack.begin();
    REQUIRE((*it)->name() == "Layer");
    ++it;
    REQUIRE((*it)->name() == "Overlay");
}

TEST_CASE("LayerStack - multiple overlays maintain insertion order", "[application][layer_stack]") {
    LayerStack stack;
    stack.push_overlay(std::make_unique<TrackingLayer>("O1"));
    stack.push_overlay(std::make_unique<TrackingLayer>("O2"));

    auto it = stack.begin();
    REQUIRE((*it)->name() == "O1");
    ++it;
    REQUIRE((*it)->name() == "O2");
}

TEST_CASE("LayerStack - layers inserted before overlays regardless of push order", "[application][layer_stack]") {
    LayerStack stack;
    stack.push_overlay(std::make_unique<TrackingLayer>("Overlay"));
    stack.push_layer(std::make_unique<TrackingLayer>("Layer"));

    auto it = stack.begin();
    REQUIRE((*it)->name() == "Layer");
    ++it;
    REQUIRE((*it)->name() == "Overlay");
}

TEST_CASE("LayerStack - pop_layer removes the given layer", "[application][layer_stack]") {
    LayerStack stack;
    auto* a = new TrackingLayer("A");
    auto* b = new TrackingLayer("B");
    stack.push_layer(std::unique_ptr<Layer>(a));
    stack.push_layer(std::unique_ptr<Layer>(b));

    stack.pop_layer(a);

    auto it = stack.begin();
    REQUIRE((*it)->name() == "B");
    ++it;
    REQUIRE(it == stack.end());
}

TEST_CASE("LayerStack - pop_overlay removes the given overlay", "[application][layer_stack]") {
    LayerStack stack;
    auto* o = new TrackingLayer("Overlay");
    stack.push_overlay(std::unique_ptr<Layer>(o));
    stack.pop_overlay(o);
    REQUIRE(stack.begin() == stack.end());
}

TEST_CASE("LayerStack - clear removes all entries", "[application][layer_stack]") {
    LayerStack stack;
    stack.push_layer(std::make_unique<TrackingLayer>("A"));
    stack.push_overlay(std::make_unique<TrackingLayer>("B"));
    stack.clear();
    REQUIRE(stack.begin() == stack.end());
}

TEST_CASE("LayerStack - reverse iteration is supported", "[application][layer_stack]") {
    LayerStack stack;
    stack.push_layer(std::make_unique<TrackingLayer>("A"));
    stack.push_layer(std::make_unique<TrackingLayer>("B"));

    std::vector<std::string> rev;
    for (auto it = stack.rbegin(); it != stack.rend(); ++it)
        rev.push_back((*it)->name());

    REQUIRE(rev[0] == "B");
    REQUIRE(rev[1] == "A");
}

TEST_CASE("Layer - name is set on construction", "[application][layer]") {
    const Layer l("MyLayer");
    REQUIRE(l.name() == "MyLayer");
}

TEST_CASE("Layer - default lifecycle hooks return without side effects", "[application][layer]") {
    Layer l("Base");
    REQUIRE(l.initialize() == true);
    REQUIRE_NOTHROW(l.update(0.016f));
    REQUIRE_NOTHROW(l.render());
    REQUIRE_NOTHROW(l.shutdown());
}
