#pragma once

#include "SDL3/SDL_surface.h"
#include <SDL3/SDL.h>

namespace CitronIO {
class Window {
  SDL_Window *sdl_window = nullptr;
  SDL_Surface *sdl_surface = nullptr;
  const char *title = nullptr;
  int width = 0, height = 0;
  bool m_shouldClose = false;

public:
  Window(const char *title, int width, int height);
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;
  ~Window();

  bool init();
  void open();
  void pollEvents();
  bool shouldClose();
  void swapBuffers();
  void close();
};

} // namespace CitronIO
