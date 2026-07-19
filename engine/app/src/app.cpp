#include "app.hpp"
#include "device.hpp"
#include <core.hpp>
#include <ecs.hpp>
#include <event.hpp>
#include <input.hpp>
#include <logger.hpp>
#include <renderer.hpp>
#include <webgpu/webgpu.hpp>
#include <x86gprintrin.h>

using namespace CitronCore;
using namespace CitronInput;
using namespace CitronECS;
using namespace CitronGraphics;

App *App::instance = nullptr;

App::App()
	: window("Citron Editor", 1280, 720, CITRON_BIND_EVENT_FN(App::onEvent)),
	  renderer(window) {
	CITRON_CORE_ASSERT(!instance, "App already exists");
	instance = this;
}

App::~App() {}

void App::init() {
	Logger::init();

	initLogSink();

	CITRON_CORE_INFO("Core logger initialized");
	CITRON_CLIENT_INFO("Client logger initialized");

	window.init();
	window.open();

	renderer.init();

	pushLayer<InputLayer>();
	pushLayer<SceneLayer>();

	onPushClientLayers();

	colorTarget = renderer.getDevice().createTexture();
	colorTargetView = renderer.getDevice().createTextureView(colorTarget);
}

void App::update() {
	while (running) {
		window.pollEvents();

		for (auto &layer : layerStack) {
			layer->onUpdate();
		}

		if (renderer.frameReady()) {
			Frame frame = renderer.beginFrame();

			RenderPass colorPass = frame.beginRenderPass(colorTarget);
			colorPass.end();

			wgpu::Texture swapchainTarget = frame.getSurfaceTexture().texture;
			RenderPass uiPass = frame.beginRenderPass(swapchainTarget);
			if (renderer.onGuiDrawCallback)
				renderer.onGuiDrawCallback(colorTargetView, uiPass);
			uiPass.end();

			renderer.endFrame(frame);
		}

		window.swapBuffers();
	}
}

void App::close() {
	renderer.end();
	running = false;
	window.close();
}

void App::onEvent(Event &e) {
	EventDispatcher dispatcher(e);
	dispatcher.dispatch<WindowCloseEvent>(
		CITRON_BIND_EVENT_FN(App::onWindowClose));

	for (auto it = layerStack.end(); it != layerStack.begin();) {
		(*--it)->onEvent(e);
		if (e.handled)
			break;
	}

	renderer.onEvent(e);
}

bool App::onWindowClose(Event &e) {
	running = false;
	return true;
}
