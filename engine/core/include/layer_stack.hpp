#pragma once

#include "layer.hpp"

#include <vector>

namespace CitronCore {
class LayerStack {
public:
  LayerStack();
  ~LayerStack();
  void pushLayer(Layer *layer);
  void popLayer(Layer *layer);

  std::vector<Layer *>::iterator begin() { return layers.begin(); }
  std::vector<Layer *>::iterator end() { return layers.end(); }

private:
  std::vector<Layer *> layers;
  std::vector<Layer *>::iterator layerInsert;
};
} // namespace CitronCore
