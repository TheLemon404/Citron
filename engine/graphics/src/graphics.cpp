#include "graphics.hpp"
#include <logger.hpp>

using namespace CitronGraphics;

void GraphicsLayer::onAttach() {
	CITRON_CORE_INFO("onAttach");
	device.aquirePlatformResources();
}

void GraphicsLayer::onDetach() { device.releasePlatformResources(); }

void GraphicsLayer::onUpdate() { device.submitCommandBuffers(); }

void GraphicsLayer::onEvent(Event &e) {}
