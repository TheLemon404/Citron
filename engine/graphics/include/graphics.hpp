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
	void constructRenderData();
	void submitRenderData();

	const Device &getDevice() const { return device; }

  private:
	Device device;
};
} // namespace CitronGraphics
