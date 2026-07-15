#pragma once

#include "event.hpp"
#include "layer.hpp"
#include "layer_stack.hpp"
#include "window.hpp"

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

class WindowEvent : public Event {
public:
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class WindowFocusEvent : public WindowEvent {
public:
  std::string toString() const override { return "WindowFocusEvent"; }
  EVENT_CLASS_TYPE(WindowFocus)
};

class WindowLostFocusEvent : public WindowEvent {
public:
  std::string toString() const override { return "WindowLostFocusEvent"; }
  EVENT_CLASS_TYPE(WindowLostFocus)
};

class WindowCloseEvent : public WindowEvent {
public:
  std::string toString() const override { return "WindowCloseEvent"; }
  EVENT_CLASS_TYPE(WindowClose)
};

class WindowResizeEvent : public WindowEvent {
public:
  WindowResizeEvent(int width, int height) : width(width), height(height) {}

  inline int getWidth() const { return width; }
  inline int getHeight() const { return height; }

  std::string toString() const override {
    std::stringstream ss;
    ss << "WindowResizeEvent: (" << width << ", " << height << ")";
    return ss.str();
  }

  EVENT_CLASS_TYPE(WindowResize)

private:
  int width, height;
};

class WindowMovedEvent : public WindowEvent {
public:
  WindowMovedEvent(int dx, int dy) : dx(dx), dy(dy) {}

  inline int getDx() const { return dx; }
  inline int getDy() const { return dy; }

  std::string toString() const override {
    std::stringstream ss;
    ss << "WindowMovedEvent: (" << dx << ", " << dy << ")";
    return ss.str();
  }
  EVENT_CLASS_TYPE(WindowMoved)

private:
  int dx, dy;
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
