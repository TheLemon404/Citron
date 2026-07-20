#include "SDL3/SDL_events.h"
#include "app.hpp"
#include "event.hpp"
#include <ecs.hpp>
#include <layer.hpp>
#include <logger.hpp>
#include <window.hpp>

using namespace CitronECS;
using namespace CitronCore;

class EditorContext {
  public:
	std::string sceneSavePath = "";

	std::shared_ptr<Scene> &getCurrentScene() { return currentScene; }
	void setCurrentScene(std::shared_ptr<Scene> scene) { currentScene = scene; }

  private:
	std::shared_ptr<Scene> currentScene = nullptr;
};

class EditorLayer : public CitronCore::Layer {
  public:
	EditorLayer() : CitronCore::Layer("EditorLayer") {}

	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	void onEvent(CitronCore::Event &e) override;

  private:
	EditorContext editorContext;

	void saveCurrentScene();
};

class Editor : public CitronCore::App {
  public:
	Editor() : CitronCore::App() {}

	virtual void onPushClientLayers() override;
};
