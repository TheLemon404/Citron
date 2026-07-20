#include "SDL3/SDL_events.h"
#include "app.hpp"
#include "event.hpp"
#include <ecs.hpp>
#include <layer.hpp>
#include <logger.hpp>
#include <window.hpp>

using namespace CitronECS;
using namespace CitronCore;

constexpr const char *CITRON_INIT_FOLDER =
	"C:/Users/vghy7/OneDrive/Desktop/Citron/";
constexpr const char *CITRON_INIT_FONT =
	"C:/Users/vghy7/OneDrive/Desktop/Citron/EngineResources/"
	"JetBrainsMono-Regular.ttf";

class EditorContext {
  public:
	EditorContext(std::string projectFilePath);
	std::string sceneSaveAssetPath = "";

	std::shared_ptr<Scene> &getCurrentScene() { return currentScene; }
	void setCurrentScene(std::shared_ptr<Scene> scene) { currentScene = scene; }
	std::string projectFilePath = "";
	std::string projectName = "";

  private:
	std::shared_ptr<Scene> currentScene = nullptr;
};

class EditorLayer : public CitronCore::Layer {
  public:
	EditorLayer();

	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	void onEvent(CitronCore::Event &e) override;
	void createProject();
	void openProject(std::string projectFilePath);

  private:
	EditorContext editorContext;

	void saveCurrentScene();
};

class Editor : public CitronCore::App {
  public:
	Editor() : CitronCore::App() {}

	virtual void onPushClientLayers() override;
};
