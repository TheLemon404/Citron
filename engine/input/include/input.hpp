#pragma once

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

class InputLayer : public Layer {
public:
  InputLayer() : Layer("InputLayer") {}
  ~InputLayer() override = default;

  void onAttach() override;
  void onDetach() override;
  void onUpdate() override;
  void onEvent(Event &e) override;

  bool isPressed(uint32_t pressable);
  bool isReleased(uint32_t pressable);
  bool isJustPressed(uint32_t pressable);
  bool isJustReleased(uint32_t pressable);

private:
  bool processKeyEvent(Event &e);
  bool processMouseEvent(Event &e);
};

} // namespace CitronInput
