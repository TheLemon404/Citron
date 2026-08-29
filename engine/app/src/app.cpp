
#include "SDL3/SDL_timer.h"
#include "clock.hpp"
#include "texture.hpp"
#include <filesystem>
#define WEBGPU_CPP_IMPLEMENTATION
#include "registry.hpp"

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
#include <instrumentor.hpp>

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
	: projectFilepath(projectFilePath), window("Citron Editor", 1280, 720, CITRON_BIND_EVENT_FN(App::onEvent)), assetManager(isRuntime, projectFilePath.parent_path(), CITRON_BIND_EVENT_FN(App::onEvent)),
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
	CITRON_PROFILE_FUNCTION();

	Logger::init();

	initLogSink();

	CITRON_CORE_INFO("Core logger initialized");
	CITRON_CLIENT_INFO("Client logger initialized");

	ECSRegistry::registerBuiltinComponents();
	ECSRegistry::registerBuiltinSystems();

	scriptingEngine.init(projectFilepath.parent_path());

	window.init();
	window.open();

	renderer.init();

	pushLayer<InputLayer>();

	RendererContext rendererContext = renderer.getContext();
}

void App::update() {
	CITRON_PROFILE_FUNCTION();
	std::vector<CitronGraphics::RenderableReferenceData> renderableData;

	while (running) {
		CITRON_PROFILE_SCOPE("App Running Loop");
		{
			CITRON_PROFILE_SCOPE("Window Poll Events");
			window.pollEvents();
		}
		{
			CITRON_PROFILE_SCOPE("Scene Update");
			sceneManager.onUpdate();
		}

		{
			CITRON_PROFILE_SCOPE("Layer Update")
			for (auto &layer : layerStack) {
				layer->onUpdate();
			}
		}

		{
			CITRON_PROFILE_SCOPE("Render") {
				CITRON_PROFILE_SCOPE("Renderable Data Extraction")
				if (sceneManager.getActiveScene()) {
					renderableData = sceneManager.getActiveScene()->extractRenderableData(assetManager);
				}
			}

			if (renderer.frameReady()) {
				Frame frame = renderer.beginFrame();
				{
					CITRON_PROFILE_SCOPE("Render Scene")
					if (!renderableData.empty()) {
						renderer.render(frame, getActiveView(), renderableData, getActiveViewSize(), renderer.lightingBufferTexture);
					}

					for (auto &layer : layerStack) {
						layer->onRender(&frame, &renderableData);
					}
				}

				{
					CITRON_PROFILE_SCOPE("End Render Frame");
					renderer.endFrame(frame);
				}
			}
		}

		Clock::tick(SDL_GetTicks());

		window.swapBuffers();
	}

	for (auto &layer : layerStack) {
		layer->onDetach();
	}
}

void App::close() {
	CITRON_PROFILE_FUNCTION();
	renderer.end();
	running = false;
	window.close();
}

void App::onEvent(Event &e) {
	CITRON_PROFILE_FUNCTION();

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
