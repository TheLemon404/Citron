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

void GraphicsContext::constructPreFrameRenderContext() {
	device.constructRenderPass();
}

void GraphicsContext::submitRenderData() { device.submitCommandBuffers(); }

void GraphicsContext::onEvent(Event &e) {
	EventDispatcher dispatcher(e);
	dispatcher.dispatch<WindowResizeEvent>(
		CITRON_BIND_EVENT_FN(GraphicsContext::onWindowResize));
}

bool GraphicsContext::onWindowResize(Event &e) {
	WindowResizeEvent &event = static_cast<WindowResizeEvent &>(e);
	device.resizeSurface(event.getWidth(), event.getHeight());

	return false;
}
