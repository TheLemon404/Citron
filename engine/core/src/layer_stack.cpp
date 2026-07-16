#include "layer_stack.hpp"

using namespace CitronCore;

LayerStack::LayerStack() { layerInsert = layers.begin(); }

LayerStack::~LayerStack() {
	for (Layer *layer : layers) {
		delete layer;
	}
}

void LayerStack::pushLayer(Layer *layer) {
	layerInsert = layers.emplace(layerInsert, layer);
}

void LayerStack::popLayer(Layer *layer) {
	auto it = std::find(layers.begin(), layers.end(), layer);
	if (it != layers.end()) {
		layers.erase(it);
		layerInsert--;
	}
}
