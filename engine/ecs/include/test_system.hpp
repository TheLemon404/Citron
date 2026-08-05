#pragma once

#include "clock.hpp"
#include "component.hpp"
#include "ecs.hpp"
#include "logger.hpp"

namespace CitronECS {

class TestSystem : public System {
  public:
	entt::entity entity;

	TestSystem() : System("TestSystem") {
	}
	virtual void init(Scene &activeScene) override {};
	virtual void start(Scene &activeScene) override {};
	virtual void update(Scene &activeScene) override {
		if (activeScene.getRegistry().any_of<TransformComponent>(entity)) {
			TransformComponent &t = activeScene.getRegistry().get<TransformComponent>(entity);
			t.position.x += Clock::getDeltaTime();
		}
	};
	virtual void onEvent(Scene &activeScene, Event &e) override {};
	virtual void end(Scene &registry) override {};
};

} // namespace CitronECS
