#pragma once

#include "spdlog/common.h"
#include <assets.hpp>
#include <ecs.hpp>
#include <event.hpp>
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
class AppEvent : public Event {
  public:
	EVENT_CLASS_CATEGORY(EventCategoryApp)
};

class AppRenderEvent : public AppEvent {
  public:
	std::string toString() const override { return "AppRenderEvent"; }
	EVENT_CLASS_TYPE(AppRender)
};

class AppTickEvent : public AppEvent {
  public:
	std::string toString() const override { return "AppTickEvent"; }
	EVENT_CLASS_TYPE(AppTick)
};

class AppUpdateEvent : public AppEvent {
  public:
	std::string toString() const override { return "AppUpdateEvent"; }
	EVENT_CLASS_TYPE(AppUpdate)
};

struct LogEntry {
	std::string message;
	uint32_t timestamp;
	std::string type;
	spdlog::level::level_enum logLevel;
};

class AppLogSink : public spdlog::sinks::base_sink<std::mutex> {
  public:
	std::vector<LogEntry> entries;

  private:
	void sink_it_(const spdlog::details::log_msg &msg) override;
	void flush_() override {}
};

class App {
  public:
	App(bool isRuntime, std::shared_ptr<AssetManager> assetManager);
	~App();

	void init();
	void update();
	void close();
	void onEvent(Event &e);

	template <typename T>
		requires std::derived_from<T, Layer>
	void pushLayer() {
		layerStack.pushLayer<T>();
		layerStack.getLayer<T>()->onAttach();
	}
	template <typename T>
		requires std::derived_from<T, Layer>
	void popLayer() {
		layerStack.getLayer<T>()->onDetach();
		layerStack.popLayer<T>();
	}

	inline static App &get() { return *instance; }
	inline bool isRunning() const { return running; }

	LayerStack &getLayerStack() { return layerStack; }
	Window &getWindow() { return window; }
	Renderer &getRenderer() { return renderer; }
	SceneManager &getSceneManager() { return sceneManager; }
	std::shared_ptr<AssetManager> getAssetManager() { return assetManager; }

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

	Renderer renderer;
	SceneManager sceneManager;
	std::shared_ptr<AssetManager> assetManager;

	bool onWindowClose(Event &e);

	bool running = true;
	LayerStack layerStack;
	static App *instance;
	wgpu::Texture colorTarget;
	wgpu::TextureView colorTargetView;

	Window window;
};
} // namespace CitronCore
