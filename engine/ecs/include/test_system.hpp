#pragma once

#include "clock.hpp"
#include "component.hpp"
#include "ecs.hpp"
#include "logger.hpp"
#include <memory>

namespace CitronECS {

class TestSystem : public System {
  public:
	float speedScaler = 1.0f;

	TestSystem() : System("TestSystem") {
	}
	virtual std::shared_ptr<System> clone() override { return std::make_shared<TestSystem>(*this); }
	virtual void init(Scene &activeScene) override {};
	virtual void start(Scene &activeScene) override {};
	virtual void update(Scene &activeScene) override {
		for (auto entity : activeScene.getRegistry().view<TransformComponent, MeshComponent>()) {
			TransformComponent &t = activeScene.getRegistry().get<TransformComponent>(entity);
			t.position.x += 0.001f * speedScaler;
		}
	};
	virtual void onEvent(Scene &activeScene, Event &e) override {};
	virtual void end(Scene &registry) override {};
};

} // namespace CitronECS
