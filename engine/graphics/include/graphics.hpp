#pragma once

#include "context.hpp"
#include <layer.hpp>

using namespace CitronCore;

namespace CitronGraphics {
class GraphicsLayer : public Layer {
public:
  GraphicsLayer(Window &window) : Layer("GraphicsLayer"), context(window) {}

  virtual void onAttach() override;
  virtual void onDetach() override;
  virtual void onUpdate() override;
  virtual void onEvent(Event &e) override;

private:
  Context context;
};
} // namespace CitronGraphics
