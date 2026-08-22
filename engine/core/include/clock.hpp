#pragma once

#include "core_exports.hpp"

namespace CitronCore {
class CITRON_CORE_API Clock {
  private:
	static float deltaTime;
	static float lastFrameTime;

  public:
	static void tick(float newFrameTime);

	static float getDeltaTime();
	static float getLastFrameTime();
};
} // namespace CitronCore
