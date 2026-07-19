#pragma once

#include <event.hpp>
#include <layer.hpp>
#include <layer_stack.hpp>
#include <logger.hpp>
#include <renderer.hpp>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>
#include <webgpu/webgpu.hpp>
#include <window.hpp>

using namespace CitronGraphics;

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

enum class LogType {
	Trace,
	Debug,
	Info,
	Warn,
	Error,
	Critical,
};

struct LogEntry {
	std::string message;
	LogType type;
};

class AppLogSink : public spdlog::sinks::base_sink<std::mutex> {
  public:
	std::vector<LogEntry> entries;

  private:
	void sink_it_(const spdlog::details::log_msg &msg) override {
		spdlog::memory_buf_t formatted;
		base_sink<std::mutex>::formatter_->format(msg, formatted);
		LogEntry e = {fmt::to_string(formatted), LogType::Info};
		entries.push_back(e);
	}
	void flush_() override {}
};

class App {
  public:
	App();
	~App();

	void init();
	virtual void onPushClientLayers() = 0;
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

	const LayerStack &getLayerStack() { return layerStack; }
	const Window &getWindow() { return window; }
	Renderer &getRenderer() { return renderer; }

	void initLogSink() {
		sink = std::make_shared<AppLogSink>();
		Logger::getCoreLogger()->sinks().push_back(sink);
		Logger::getClientLogger()->sinks().push_back(sink);
	}

	AppLogSink *getLogSink() { return sink.get(); }

  private:
	std::shared_ptr<AppLogSink> sink = nullptr;

	Renderer renderer;
	bool onWindowClose(Event &e);

	bool running = true;
	LayerStack layerStack;
	static App *instance;
	wgpu::Texture colorTarget;
	wgpu::TextureView colorTargetView;

	Window window;
};
} // namespace CitronCore
