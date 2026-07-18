#pragma once

#include <event.hpp>
#include <layer.hpp>
#include <layer_stack.hpp>
#include <renderer.hpp>
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

  private:
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
