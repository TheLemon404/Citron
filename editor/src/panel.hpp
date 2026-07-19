#pragma once

#include <event.hpp>

using namespace CitronCore;

class Panel {
  public:
	Panel(const std::string &name) : name(name) {};
	virtual ~Panel() = default;

	virtual void onAttach() {};
	virtual void onDetach() {};
	virtual void onUpdate() {};
	virtual void onDraw() {};
	virtual void onEvent(Event &e) {};

	inline const std::string &getName() const { return name; }

  protected:
	const std::string name;
};

class ConsolePanel : public Panel {
  public:
	ConsolePanel() : Panel("Console") {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;
};
