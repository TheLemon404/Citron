#pragma once

#include "citron_exports.hpp"
#include "layer.hpp"
#include <SDL3/SDL.h>
#include <event.hpp>
#include <logger.hpp>
#include <unordered_map>
#include <window.hpp>

using namespace CitronCore;

namespace CitronInput {
enum class PressableInputState {
	PRESSED,
	RELEASED,
	JUST_PRESSED,
	JUST_RELEASED
};

class CITRON_INPUT_API InputLayer : public Layer {
  public:
	InputLayer() : Layer("InputLayer") {}
	~InputLayer() override = default;

	void onAttach() override;
	void onDetach() override;
	void onRender(void *frame) override {};
	void onUpdate() override;
	void onEvent(Event &e) override;

	bool isPressed(uint32_t pressable);
	bool isReleased(uint32_t pressable);
	bool isJustPressed(uint32_t pressable);
	bool isJustReleased(uint32_t pressable);

  private:
	bool processKeyPressedEvent(Event &e);
	bool processKeyReleasedEvent(Event &e);
	bool processMouseEvent(Event &e);

	static std::unordered_map<uint64_t, PressableInputState> pressedInputs;
};

} // namespace CitronInput
