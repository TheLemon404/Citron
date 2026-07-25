#include "app.hpp"
#include "assets.hpp"
#include "device.hpp"
#include "pipeline.hpp"
#include "spdlog/common.h"
#include <core.hpp>
#include <ctime>
#include <ecs.hpp>
#include <event.hpp>
#include <input.hpp>
#include <logger.hpp>
#include <memory>
#include <renderer.hpp>
#include <shader.hpp>
#include <string>
#include <string_view>
#include <webgpu/webgpu.hpp>
#include <x86gprintrin.h>

using namespace CitronCore;
using namespace CitronInput;
using namespace CitronECS;
using namespace CitronGraphics;

App *App::instance = nullptr;

void AppLogSink::sink_it_(const spdlog::details::log_msg &msg) {
	std::string_view message = (std::string_view)msg.payload;
	uint32_t time = spdlog::log_clock::to_time_t(msg.time);
	std::string type = "";
	switch (msg.level) {
	case spdlog::level::trace:
		type = "Trace";
		break;
	case spdlog::level::debug:
		type = "Debug";
		break;
	case spdlog::level::info:
		type = "Info";
		break;
	case spdlog::level::warn:
		type = "Warning";
		break;
	case spdlog::level::err:
		type = "Error";
		break;
	case spdlog::level::critical:
		type = "Critical";
		break;
	case spdlog::level::off:
		type = "Off";
		break;
	}
	LogEntry e = {std::string(message), time, type, msg.level};
	entries.push_back(e);
	if (entries.size() > 64)
		entries.erase(entries.begin());
}

App::App(bool isRuntime, std::filesystem::path projectFilePath)
	: window("Citron Editor", 1280, 720, CITRON_BIND_EVENT_FN(App::onEvent)), assetManager(isRuntime, projectFilePath.parent_path()),
	  renderer(window, assetManager),
	  isRuntimeMode(isRuntime), sceneManager(assetManager) {
	CITRON_CORE_ASSERT(!instance, "App already exists");
	instance = this;

	materialImporter = std::make_shared<MaterialImporter>();
	shaderImporter = std::make_shared<ShaderImporter>(renderer.getDevice());
	assetManager.registerAssetImporter(AssetType::MATERIAL, materialImporter);
	assetManager.registerAssetImporter(AssetType::SHADER, shaderImporter);
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

	colorTarget = renderer.getDevice().createEmptyRenderTargetTexture();
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
			std::shared_ptr<Shader> shader = assetManager.getAsset<Shader>(7422933780722231991);
			std::shared_ptr<Pipeline> pipeline = renderer.getPipeline(shader, colorTarget);
			if (pipeline) {
				colorPass.setPipeline(pipeline);
				colorPass.draw();
			}
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

	for (auto &layer : layerStack) {
		layer->onDetach();
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
