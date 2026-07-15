#pragma once

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

class Input {
  static std::unordered_map<uint32_t, PressableInputState> pressableInputs;

public:
  static bool isPressed(uint32_t pressable);
  static bool isReleased(uint32_t pressable);
  static bool isJustPressed(uint32_t pressable);
  static bool isJustReleased(uint32_t pressable);
};

} // namespace CitronInput
