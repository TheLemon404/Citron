#pragma once

#include "citron_exports.hpp"

#include "mesh.hpp"
#include "spdlog/common.h"
#include "texture.hpp"
#include <assets.hpp>
#include <ecs.hpp>
#include <event.hpp>
#include <filesystem>
#include <layer.hpp>
#include <layer_stack.hpp>
#include <logger.hpp>
#include <memory>
#include <renderer.hpp>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>
#include <webgpu/webgpu.hpp>
#include <window.hpp>

using namespace CitronGraphics;
using namespace CitronECS;
using namespace CitronAssets;

namespace CitronCore {
class CITRON_APP_API AppEvent : public Event {
  public:
	EVENT_CLASS_CATEGORY(EventCategoryApp)
};

class CITRON_APP_API AppRenderEvent : public AppEvent {
  public:
	std::string toString() const override { return "AppRenderEvent"; }
	EVENT_CLASS_TYPE(AppRender)
};

class CITRON_APP_API AppTickEvent : public AppEvent {
  public:
	std::string toString() const override { return "AppTickEvent"; }
	EVENT_CLASS_TYPE(AppTick)
};

class CITRON_APP_APIAppUpdateEvent : public AppEvent {
  public:
	std::string toString() const override { return "AppUpdateEvent"; }
	EVENT_CLASS_TYPE(AppUpdate)
};

struct CITRON_APP_API LogEntry {
	std::string message;
	uint32_t timestamp;
	std::string type;
	spdlog::level::level_enum logLevel;
};

class CITRON_APP_API AppLogSink : public spdlog::sinks::base_sink<std::mutex> {
  public:
	std::vector<LogEntry> entries;

  private:
	void sink_it_(const spdlog::details::log_msg &msg) override;
	void flush_() override {}
};

struct CITRON_APP_API AppContext {
	CitronCore::Window &window;
	CitronGraphics::Renderer &renderer;
	CitronAssets::AssetManager &assetManager;
	CitronECS::SceneManager &sceneManager;
};

class CITRON_APP_API App {
  public:
	App(bool isRuntime, std::filesystem::path projectFilePath);
	~App();

	void init();
	void update();
	void close();
	void onEvent(Event &e);

	template <typename T, typename... Args>
		requires std::derived_from<T, Layer>
	void pushLayer(Args &&...args) {
		layerStack.pushLayer<T>(std::forward<Args>(args)...);
		layerStack.getLayer<T>()->onAttach();
	}
	template <typename T>
		requires std::derived_from<T, Layer>
	void popLayer() {
		layerStack.getLayer<T>()->onDetach();
		layerStack.popLayer<T>();
	}
	template <typename T>
	T *getLayer() {
		return static_cast<T *>(layerStack.getLayer<T>());
	}

	static App &get() { return *instance; }
	bool isRunning() const { return running; }

	AppContext getContext() {
		return {window, renderer, assetManager, sceneManager};
	}

	void initLogSink() {
		sink = std::make_shared<AppLogSink>();
		Logger::getCoreLogger()->sinks().push_back(sink);
		Logger::getClientLogger()->sinks().push_back(sink);
	}

	AppLogSink *getLogSink() { return sink.get(); }

	const bool isRuntime() { return isRuntimeMode; }

  protected:
	const bool isRuntimeMode = false;

	std::shared_ptr<AppLogSink> sink = nullptr;

	Window window;
	AssetManager assetManager;
	SceneManager sceneManager;
	Renderer renderer;

	std::vector<CitronGraphics::RenderObject> extractRenderObjects();

	bool onWindowClose(Event &e);

	bool running = true;
	LayerStack layerStack;
	static App *instance;
};
} // namespace CitronCore
