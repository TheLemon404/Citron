#include "graphics.hpp"
#include <logger.hpp>

using namespace CitronGraphics;

void GraphicsContext::init() {
	CITRON_CORE_INFO("onAttach");
	device.aquirePlatformResources();
}

void GraphicsContext::end() { device.releasePlatformResources(); }

void GraphicsContext::constructRenderData() {}

void GraphicsContext::submitRenderData() { device.submitCommandBuffers(); }
