#include "graphics.hpp"
#include "window.hpp"
#include <event.hpp>
#include <logger.hpp>

using namespace CitronGraphics;

void GraphicsContext::init() {
	CITRON_CORE_INFO("onAttach");
	device.aquirePlatformResources();
}

void GraphicsContext::end() { device.releasePlatformResources(); }

bool GraphicsContext::constructRenderContext() {
	return device.constructRenderPass();
}

void GraphicsContext::submitRenderData() { device.submitCommandBuffers(); }

void GraphicsContext::onEvent(Event &e) {}
