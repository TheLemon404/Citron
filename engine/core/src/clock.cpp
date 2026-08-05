#include "clock.hpp"

using namespace CitronCore;

float Clock::deltaTime = 0.0f;
float Clock::lastFrameTime = 0.0f;

void Clock::tick(float newFrameTime) {
	deltaTime = newFrameTime - lastFrameTime;
	lastFrameTime = newFrameTime;
}

float Clock::getDeltaTime() {
	return deltaTime;
}

float Clock::getLastFrameTime() {
	return lastFrameTime;
}
