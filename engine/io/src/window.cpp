#include "window.hpp"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_messagebox.h"
#include "SDL3/SDL_video.h"

using namespace CitronIO;

Window::Window(const char *title, int width, int height)
    : title(title), width(width), height(height) {}

Window::~Window() {
  if (sdl_window && SDL_WasInit(SDL_INIT_VIDEO)) {
    SDL_DestroyWindow(sdl_window);
  }
}

bool Window::init() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                             "Error initializing SDL3", nullptr);
    return false;
  }
  return true;
}

void Window::open() {
  sdl_window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
  if (!sdl_window) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                             "Error creating SDL window", nullptr);
    close();
    return;
  }

  sdl_surface = SDL_GetWindowSurface(sdl_window);
  if (!sdl_surface) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                             "Error getting SDL window surface", nullptr);
    close();
    return;
  }
  SDL_UpdateWindowSurface(sdl_window);
}

void Window::pollEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      m_shouldClose = true;
    }
  }
}

bool Window::shouldClose() { return m_shouldClose; }

void Window::swapBuffers() {}

void Window::close() {
  if (sdl_window) {
    SDL_DestroyWindow(sdl_window);
    sdl_window = nullptr;
  }

  SDL_Quit();
}
