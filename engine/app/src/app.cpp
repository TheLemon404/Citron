#include "app.hpp"
#include "assets.hpp"
#include "device.hpp"
#include "glm/fwd.hpp"
#include "mesh.hpp"
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
	: window("Citron Editor", 1280, 720, CITRON_BIND_EVENT_FN(App::onEvent)), assetManager(isRuntime, projectFilePath.parent_path(), CITRON_BIND_EVENT_FN(App::onEvent)),
	  renderer(window, assetManager),
	  isRuntimeMode(isRuntime), sceneManager(assetManager) {
	CITRON_CORE_ASSERT(!instance, "App already exists");
	instance = this;

	RendererContext rendererContext = renderer.getContext();
	assetManager.registerAssetImporter(AssetType::MATERIAL, std::make_shared<MaterialImporter>(rendererContext.device));
	assetManager.registerAssetImporter(AssetType::SHADER, std::make_shared<ShaderImporter>(rendererContext.device));
	assetManager.registerAssetImporter(AssetType::MESH, std::make_shared<MeshImporter>(rendererContext.device));
	assetManager.registerAssetImporter(AssetType::TEXTURE, std::make_shared<TextureImporter>(rendererContext.device));
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

	RendererContext rendererContext = renderer.getContext();
}

void App::update() {
	std::vector<Vertex> triPositions = {
		// Define a first triangle:
		Vertex(-0.5, -0.5, 0.0),
		Vertex(+0.5, -0.5, 0.0),
		Vertex(+0.0, +0.5, 0.0),

		// Add a second triangle:
		Vertex(-0.55f, -0.5, 0.0),
		Vertex(-0.05f, +0.5, 0.0),
		Vertex(-0.55f, +0.5, 0.0)};
	std::vector<uint32_t> triIndices = {
		0,
		1,
		2,
		3,
		4,
		5,
	};
	std::shared_ptr<Mesh> geometry = std::make_shared<Mesh>(triPositions, triIndices, renderer.getContext().device);

	while (running) {
		window.pollEvents();
		sceneManager.onUpdate();

		for (auto &layer : layerStack) {
			layer->onUpdate();
		}

		if (renderer.frameReady()) {
			Frame frame = renderer.beginFrame();
			if (sceneManager.getActiveScene()) {
				RenderPass colorPass = frame.beginRenderPass(renderer.getColorTarget());
				std::vector<uint64_t> meshUUIDs = sceneManager.getActiveScene()->extractMeshes(assetManager);
				std::vector<uint64_t> materialUUIDs = sceneManager.getActiveScene()->extractMaterials(assetManager);
				colorPass.drawRenderData(meshUUIDs, materialUUIDs);
				colorPass.end();
			}

			wgpu::Texture surfaceTexture = renderer.getContext().device.getCurrentSurfaceTexture().texture;
			RenderPass uiPass = frame.beginRenderPass(surfaceTexture);
			if (renderer.onGuiDrawCallback)
				renderer.onGuiDrawCallback(renderer.getColorTargetView(), uiPass);
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

	sceneManager.onEvent(e);
	renderer.onEvent(e);
}

bool App::onWindowClose(Event &e) {
	running = false;
	return true;
}
