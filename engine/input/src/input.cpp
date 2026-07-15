#include "input.hpp"
#include "SDL3/SDL_events.h"

using namespace CitronInput;

std::unordered_map<uint32_t, PressableInputState> Input::pressableInputs{};
