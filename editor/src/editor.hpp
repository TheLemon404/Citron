#include "app.hpp"
#include <ecs.hpp>
#include <layer.hpp>
#include <logger.hpp>
#include <window.hpp>

using namespace CitronECS;

class EditorContext {
  public:
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
};

class Editor : public CitronCore::App {
  public:
	Editor() : CitronCore::App() {}

	virtual void onPushClientLayers() override;
};
