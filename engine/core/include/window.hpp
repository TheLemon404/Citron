#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_surface.h"
#include <SDL3/SDL.h>
#include <event.hpp>
#include <functional>
#include <sstream>

namespace CitronCore {

class WindowEvent : public Event {
  public:
	WindowEvent(SDL_Event *e) : Event(e) {}
	EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class WindowFocusEvent : public WindowEvent {
  public:
	WindowFocusEvent(SDL_Event *e) : WindowEvent(e) {}
	std::string toString() const override { return "WindowFocusEvent"; }
	EVENT_CLASS_TYPE(WindowFocus)
};

class WindowLostFocusEvent : public WindowEvent {
  public:
	WindowLostFocusEvent(SDL_Event *e) : WindowEvent(e) {}
	std::string toString() const override { return "WindowLostFocusEvent"; }
	EVENT_CLASS_TYPE(WindowLostFocus)
};

class WindowCloseEvent : public WindowEvent {
  public:
	WindowCloseEvent(SDL_Event *e) : WindowEvent(e) {}
	std::string toString() const override { return "WindowCloseEvent"; }
	EVENT_CLASS_TYPE(WindowClose)
};

class WindowResizeEvent : public WindowEvent {
  public:
	WindowResizeEvent(SDL_Event *e, int width, int height)
		: WindowEvent(e), width(width), height(height) {}

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
	WindowMovedEvent(SDL_Event *e, int dx, int dy)
		: WindowEvent(e), dx(dx), dy(dy) {}

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

class Window {
	SDL_Window *sdl_window = nullptr;
	SDL_Surface *sdl_surface = nullptr;
	const char *title = nullptr;
	int width = 0, height = 0;

  public:
	using EventCallbackFn = std::function<void(Event &)>;

	Window(const char *title, int width, int height,
		   EventCallbackFn eventCallback);
	Window(const Window &) = delete;
	Window &operator=(const Window &) = delete;
	~Window();

	inline void setEventCallback(const EventCallbackFn &callback) {
		eventCallback = callback;
	}

	void setName(std::string &name);

	bool init();
	void open();
	void pollEvents();
	void swapBuffers();
	void close();

	void *getSDLWindow() const { return sdl_window; }
	void *getSDLSurface() const { return sdl_surface; }

	int getWidth() const { return width; }
	int getHeight() const { return height; }

  private:
	EventCallbackFn eventCallback = nullptr;
};

} // namespace CitronCore
