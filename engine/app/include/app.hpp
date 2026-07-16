#pragma once

#include <event.hpp>
#include <layer.hpp>
#include <layer_stack.hpp>
#include <window.hpp>

#include <sstream>

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

class App {
  public:
	App();
	~App();

	void init();
	void update();
	void close();
	void onEvent(Event &e);

	void pushLayer(Layer *layer);
	void popLayer(Layer *layer);

	inline static App &get() { return *instance; }
	inline bool isRunning() const { return running; }

  private:
	bool onWindowClose(Event &e);

	bool running = true;
	LayerStack layerStack;
	static App *instance;

	Window window;
};
} // namespace CitronCore
