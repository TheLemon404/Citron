#pragma once

#include "panel.hpp"

class OutlinerPanel : public Panel {
  public:
	OutlinerPanel(AppContext appContext) : Panel("Outliner", appContext) {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;

  private:
	void showEntityChildTree(entt::entity entity,
							 std::shared_ptr<Scene> &scenecontext);
	bool pendingCreateEntity = false;
	UUID pendingCreateEntityParent = UUID::nullID;
	UUID pendingDeleteEntity = UUID::nullID;
	entt::entity shiftSelectStartEntity = entt::null;
	std::shared_ptr<System> pendingDeleteSystem = nullptr;
};
