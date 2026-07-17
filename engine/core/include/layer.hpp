#pragma once

#include "event.hpp"
#include <string>

namespace CitronCore {
class Layer {
  public:
	Layer(const std::string &debugName) : debugName(debugName) {};
	virtual ~Layer() = default;

	virtual void onAttach() = 0;
	virtual void onDetach() = 0;
	virtual void onUpdate() = 0;
	virtual void onEvent(Event &e) = 0;
	virtual void onRender() {};

	inline const std::string &getName() const { return debugName; }

  protected:
	const std::string debugName;
};
} // namespace CitronCore
