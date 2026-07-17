#include "app.hpp"
#include <concepts>
#include <core.hpp>
#include <ecs.hpp>
#include <event.hpp>
#include <graphics.hpp>
#include <input.hpp>
#include <logger.hpp>
#include <x86gprintrin.h>

using namespace CitronCore;
using namespace CitronInput;
using namespace CitronECS;
using namespace CitronGraphics;

App *App::instance = nullptr;

App::App()
	: window("Citron Editor", 800, 600, CITRON_BIND_EVENT_FN(App::onEvent)),
	  graphicsContext(window) {
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

	graphicsContext.init();

	pushLayer<InputLayer>();
	pushLayer<SceneLayer>();

	onPushClientLayers();
}

void App::update() {
	while (running) {
		window.pollEvents();

		for (auto &layer : layerStack) {
			layer->onUpdate();
		}

		if (graphicsContext.constructRenderContext()) {
			for (auto &layer : layerStack) {
				layer->onRender();
			}
			graphicsContext.submitRenderData();
		}

		window.swapBuffers();
	}
}

void App::close() {
	graphicsContext.end();
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

	graphicsContext.onEvent(e);
}

bool App::onWindowClose(Event &e) {
	running = false;
	return true;
}
