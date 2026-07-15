#include "window.hpp"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_messagebox.h"
#include "SDL3/SDL_oldnames.h"
#include "SDL3/SDL_video.h"
#include "keyboard.hpp"
#include "mouse.hpp"
#include <logger.hpp>

using namespace CitronCore;

Window::Window(const char *title, int width, int height,
               EventCallbackFn eventCallback)
    : title(title), width(width), height(height), eventCallback(eventCallback) {
}

Window::~Window() {
  if (sdl_window && SDL_WasInit(SDL_INIT_VIDEO)) {
    SDL_DestroyWindow(sdl_window);
  }
}

bool Window::init() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    CITRON_CORE_CRITICAL("Error initializing SDL3");
    return false;
  }
  return true;
}

void Window::open() {
  sdl_window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
  if (!sdl_window) {
    CITRON_CORE_CRITICAL("Error initializing SDL window");
    close();
    return;
  }

  sdl_surface = SDL_GetWindowSurface(sdl_window);
  if (!sdl_surface) {
    CITRON_CORE_CRITICAL("Error getting SDL window surface");

    close();
    return;
  }
  SDL_UpdateWindowSurface(sdl_window);
}

void Window::pollEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_EVENT_QUIT: {
      WindowCloseEvent closeEvent = WindowCloseEvent();
      eventCallback(closeEvent);
      break;
    }
    case SDL_EVENT_WINDOW_RESIZED: {
      WindowResizeEvent resizeEvent =
          WindowResizeEvent(event.window.data1, event.window.data2);
      eventCallback(resizeEvent);
      break;
    }
    case SDL_EVENT_WINDOW_FOCUS_GAINED: {
      WindowFocusEvent focusEvent = WindowFocusEvent();
      eventCallback(focusEvent);
      break;
    }
    case SDL_EVENT_WINDOW_FOCUS_LOST: {
      WindowLostFocusEvent focusEvent = WindowLostFocusEvent();
      eventCallback(focusEvent);
      break;
    }
    case SDL_EVENT_KEY_DOWN: {
      if (event.key.repeat == 0) {
        KeyJustPressedEvent pressedEvent = KeyJustPressedEvent(event.key.key);
        eventCallback(pressedEvent);
      } else {
        KeyPressedEvent pressedEvent =
            KeyPressedEvent(event.key.key, event.key.repeat);
        eventCallback(pressedEvent);
      }
      break;
    }
    case SDL_EVENT_KEY_UP: {
      // TODO: add key just released logic
      KeyReleasedEvent releasedEvent = KeyReleasedEvent(event.key.key);
      eventCallback(releasedEvent);
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      MouseMovedEvent motionEvent =
          MouseMovedEvent(event.motion.x, event.motion.y);
      eventCallback(motionEvent);
      break;
    }
    case SDL_EVENT_MOUSE_WHEEL: {
      MouseScrolledEvent scrolledEvent =
          MouseScrolledEvent(event.motion.x, event.motion.y);
      eventCallback(scrolledEvent);
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      MouseButtonPressedEvent pressedEvent =
          MouseButtonPressedEvent(event.button.button);
      eventCallback(pressedEvent);
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      MouseButtonReleasedEvent releasedEvent =
          MouseButtonReleasedEvent(event.button.button);
      eventCallback(releasedEvent);
      break;
    }
    }
  }
}

void Window::swapBuffers() {}

void Window::close() {
  WindowCloseEvent event = WindowCloseEvent();

  if (sdl_window) {
    SDL_DestroyWindow(sdl_window);
    sdl_window = nullptr;
  }

  SDL_Quit();
}
