#pragma once

#include "device.hpp"
#include <layer.hpp>

using namespace CitronCore;

namespace CitronGraphics {
class GraphicsLayer : public Layer {
  public:
	GraphicsLayer(Window &window) : Layer("GraphicsLayer"), device(window) {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onEvent(Event &e) override;

  private:
	Device device;
};
} // namespace CitronGraphics
