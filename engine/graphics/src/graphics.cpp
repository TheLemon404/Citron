#include "graphics.hpp"
#include <logger.hpp>

using namespace CitronGraphics;

void GraphicsLayer::onAttach() {
	CITRON_CORE_INFO("onAttach");
	context.aquirePlatformResources();
}

void GraphicsLayer::onDetach() {}

void GraphicsLayer::onUpdate() {}

void GraphicsLayer::onEvent(Event &e) {}
