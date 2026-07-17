#pragma once

#include <layer.hpp>

using namespace CitronCore;

class GuiLayer : public Layer {
  public:
	GuiLayer() : Layer("GuiLayer") {};
	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	void onEvent(Event &e) override;
};
