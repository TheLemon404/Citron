#include "app.hpp"
#include "device.hpp"
#include <concepts>
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
	: window("Citron Editor", 800, 600, CITRON_BIND_EVENT_FN(App::onEvent)),
	  renderer(window) {
	CITRON_CORE_ASSERT(!instance, "App already exists");
	instance = this;
}

App::~App() {}

void App::init() {
	Logger::init();
	CITRON_CORE_INFO("Core logger initialized");
	CITRON_CLIENT_INFO("Client logger initialized");

	window.init();
	window.open();

	renderer.init();

	pushLayer<InputLayer>();
	pushLayer<SceneLayer>();

	onPushClientLayers();

	wgpu::TextureDescriptor colorTargetDesc = {};
	colorTargetDesc.label = wgpu::StringView("colorTarget");
	colorTargetDesc.dimension = wgpu::TextureDimension::_2D;
	colorTargetDesc.size.width = window.getWidth();
	colorTargetDesc.size.height = window.getHeight();
	colorTargetDesc.size.depthOrArrayLayers = 1;
	colorTargetDesc.mipLevelCount = 1;
	colorTargetDesc.sampleCount = 1;
	colorTargetDesc.format =
		renderer.getDevice().getWGPUPreferredSurfaceFormat();
	colorTargetDesc.usage = wgpu::TextureUsage::RenderAttachment |
							wgpu::TextureUsage::TextureBinding;
	colorTarget =
		renderer.getDevice().getWGPUDevice().createTexture(colorTargetDesc);

	colorTargetView = colorTarget.createView();
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
