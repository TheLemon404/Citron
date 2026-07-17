#pragma once

#include "device.hpp"
#include <layer.hpp>

using namespace CitronCore;

namespace CitronGraphics {
class GraphicsContext {
  public:
	GraphicsContext(Window &window) : device(window) {}

	void init();
	void end();
	bool constructRenderContext();
	void submitRenderData();
	void onEvent(Event &e);

	const Device &getDevice() const { return device; }

  private:
	Device device;
};
} // namespace CitronGraphics
