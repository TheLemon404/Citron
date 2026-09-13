#pragma once

#include "panel.hpp"

class ConsolePanel : public Panel {
  public:
	ConsolePanel(AppContext appContext) : Panel("Console", appContext) {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;
};
