#include "input.hpp"
#include "SDL3/SDL_events.h"
#include "event.hpp"
#include "keyboard.hpp"
#include "mouse.hpp"
#include <print>

using namespace CitronInput;
using namespace CitronCore;

void InputLayer::onAttach() {}

void InputLayer::onDetach() {}

void InputLayer::onUpdate() {}

void InputLayer::onEvent(Event &e) {
  EventDispatcher dispatcher(e);
  dispatcher.dispatch<KeyPressedEvent>(
      CITRON_BIND_EVENT_FN(InputLayer::processKeyEvent));
  dispatcher.dispatch<KeyReleasedEvent>(
      CITRON_BIND_EVENT_FN(InputLayer::processKeyEvent));
  dispatcher.dispatch<MouseButtonPressedEvent>(
      CITRON_BIND_EVENT_FN(InputLayer::processMouseEvent));
  dispatcher.dispatch<MouseButtonReleasedEvent>(
      CITRON_BIND_EVENT_FN(InputLayer::processMouseEvent));
  dispatcher.dispatch<MouseMovedEvent>(
      CITRON_BIND_EVENT_FN(InputLayer::processMouseEvent));
  dispatcher.dispatch<MouseScrolledEvent>(
      CITRON_BIND_EVENT_FN(InputLayer::processMouseEvent));
}

bool InputLayer::processKeyEvent(Event &e) { return false; }

bool InputLayer::processMouseEvent(Event &e) { return false; }
