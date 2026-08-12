#pragma once

#include "app.hpp"
#include "entt/entity/entity.hpp"
#include "entt/entity/fwd.hpp"
#include "event.hpp"
#include "view.hpp"
#include <ecs.hpp>
#include <layer.hpp>
#include <logger.hpp>
#include <variant>
#include <window.hpp>

using namespace CitronECS;
using namespace CitronCore;

constexpr const char *CITRON_PROGRAM_FOLDER = "C:/Citron";

enum class EditorPlaymodeState {
	Playing,
	Paused,
	Stopped,
};

class EditorPlaymodeEvent : public Event {
  public:
	EVENT_CLASS_CATEGORY(EventCategoryApp)
	EVENT_CLASS_TYPE(None)

	const EditorPlaymodeState state;
	EditorPlaymodeEvent(EditorPlaymodeState state) : Event(nullptr), state(state) {}
};

class EditorContext {
  public:
	std::filesystem::path currentlyEditedSceneAssetPath = "";

	std::variant<entt::entity, std::shared_ptr<System>> &getCurrentlySelectedItem() {
		return currentlySelectedItem;
	}

	void setCurrentlySelectedItem(const entt::entity entity) {
		currentlySelectedItem = entity;
	}
	void setCurrentlySelectedItem(const std::shared_ptr<System> &system) {
		currentlySelectedItem = system;
	}

	std::filesystem::path projectFilePath = "";

	EditorPlaymodeState getPlaymodeState() const { return playmodeState; }

	void setPlaymodeState(EditorPlaymodeState state) {
		playmodeState = state;
	}

  private:
	EditorPlaymodeState playmodeState = EditorPlaymodeState::Stopped;
	std::variant<entt::entity, std::shared_ptr<System>> currentlySelectedItem;
};

class Editor : public CitronCore::App {
  public:
	Editor(const std::string &projectFilePath);
	inline static Editor &get() { return (Editor &)App::get(); }

	EditorContext &getEditorContext() { return editorContext; }

	void startPlaying();
	void stopPlaying();
	void pausePlaying();
	void resumePlaying();

	void init();
	void close();
	void update();

	bool openScene(std::string sceneAssetPath);
	bool createScene();

	bool openProject(std::string projectFilePath);

	void saveCurrentScene();

	PerspectiveView editorView;

	glm::ivec2 getActiveViewSize() override;

  private:
	std::shared_ptr<Scene> temporaryPlaymodeSceneHolder = nullptr;
	EditorContext editorContext;
};
