#pragma once

#include "SDL3/SDL_surface.h"
#include <SDL3/SDL.h>
#include <event.hpp>
#include <functional>
#include <sstream>

namespace CitronCore {
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

  bool init();
  void open();
  void pollEvents();
  void swapBuffers();
  void close();

private:
  EventCallbackFn eventCallback = nullptr;
};

} // namespace CitronCore
