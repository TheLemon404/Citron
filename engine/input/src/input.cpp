#include "input.hpp"
#include "SDL3/SDL_events.h"
#include "event.hpp"
#include "keyboard.hpp"
#include "mouse.hpp"
#include <print>
#include <unordered_map>

using namespace CitronInput;
using namespace CitronCore;

std::unordered_map<uint64_t, PressableInputState> InputLayer::pressedInputs;

void InputLayer::onAttach() {}

void InputLayer::onDetach() {}

void InputLayer::onUpdate() {}

void InputLayer::onEvent(Event &e) {
	EventDispatcher dispatcher(e);
	dispatcher.dispatch<KeyPressedEvent>(
		CITRON_BIND_EVENT_FN(InputLayer::processKeyPressedEvent));
	dispatcher.dispatch<KeyReleasedEvent>(
		CITRON_BIND_EVENT_FN(InputLayer::processKeyReleasedEvent));
	dispatcher.dispatch<MouseButtonPressedEvent>(
		CITRON_BIND_EVENT_FN(InputLayer::processMouseEvent));
	dispatcher.dispatch<MouseButtonReleasedEvent>(
		CITRON_BIND_EVENT_FN(InputLayer::processMouseEvent));
	dispatcher.dispatch<MouseMovedEvent>(
		CITRON_BIND_EVENT_FN(InputLayer::processMouseEvent));
	dispatcher.dispatch<MouseScrolledEvent>(
		CITRON_BIND_EVENT_FN(InputLayer::processMouseEvent));
}

bool InputLayer::processKeyPressedEvent(Event &e) {
	KeyPressedEvent *keyPressedEvent = static_cast<KeyPressedEvent *>(&e);
	uint64_t keyCode = keyPressedEvent->getKeycode();
	pressedInputs[keyCode] = PressableInputState::PRESSED;
	return false;
}

bool InputLayer::processKeyReleasedEvent(Event &e) {
	KeyReleasedEvent *keyReleasedEvent = static_cast<KeyReleasedEvent *>(&e);
	uint64_t keyCode = keyReleasedEvent->getKeycode();
	pressedInputs[keyCode] = PressableInputState::RELEASED;
	return false;
}

bool InputLayer::processMouseEvent(Event &e) { return false; }

bool InputLayer::isPressed(uint32_t pressable) {
	return pressedInputs.contains(pressable) && (pressedInputs[pressable] == PressableInputState::PRESSED || pressedInputs[pressable] == PressableInputState::JUST_PRESSED);
}

bool InputLayer::isReleased(uint32_t pressable) {
	return pressedInputs.contains(pressable) && (pressedInputs[pressable] == PressableInputState::RELEASED || pressedInputs[pressable] == PressableInputState::JUST_RELEASED);
}

bool InputLayer::isJustPressed(uint32_t pressable) {
	return pressedInputs.contains(pressable) && pressedInputs[pressable] == PressableInputState::JUST_PRESSED;
}

bool InputLayer::isJustReleased(uint32_t pressable) {
	return pressedInputs.contains(pressable) && pressedInputs[pressable] == PressableInputState::JUST_RELEASED;
}
