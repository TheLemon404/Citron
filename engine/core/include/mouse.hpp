#pragma once

#include "SDL3/SDL_events.h"
#include "event.hpp"
#include <sstream>

namespace CitronCore {
class CITRON_CORE_API MouseEvent : public Event {
  public:
	MouseEvent(SDL_Event *e) : Event(e) {}

	EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
};

class CITRON_CORE_API MouseScrolledEvent : public MouseEvent {
  public:
	MouseScrolledEvent(SDL_Event *e, double dx, double dy)
		: MouseEvent(e), dx(dx), dy(dy) {}

	inline double getDx() const { return dx; }
	inline double getDy() const { return dy; }

	std::string toString() const override {
		std::stringstream ss;
		ss << "MouseScrolledEvent: (" << dx << "," << dy << ")";
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseScrolled)

  private:
	double dx;
	double dy;
};

class CITRON_CORE_API MouseMovedEvent : public MouseEvent {
  public:
	MouseMovedEvent(SDL_Event *e, double dx, double dy)
		: MouseEvent(e), dx(dx), dy(dy) {}

	inline double getDx() const { return dx; }
	inline double getDy() const { return dy; }

	std::string toString() const override {
		std::stringstream ss;
		ss << "MouseMovedEvent: (" << dx << "," << dy << ")";
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseMoved)

  private:
	double dx;
	double dy;
};

class CITRON_CORE_API MouseButtonEvent : public MouseEvent {
  public:
	MouseButtonEvent(SDL_Event *e) : MouseEvent(e) {}

	EVENT_CLASS_CATEGORY(EventCategoryMouseButton | EventCategoryMouse |
						 EventCategoryInput)
};

class CITRON_CORE_API MouseButtonPressedEvent : public MouseButtonEvent {
  public:
	MouseButtonPressedEvent(SDL_Event *e, int button)
		: MouseButtonEvent(e), button(button) {}

	inline int getButton() const { return button; }

	std::string toString() const override {
		std::stringstream ss;
		ss << "MouseButtonPressedEvent: " << button;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseButtonPressed)

  private:
	int button;
};

class CITRON_CORE_API MouseButtonReleasedEvent : public MouseButtonEvent {
  public:
	MouseButtonReleasedEvent(SDL_Event *e, int button)
		: MouseButtonEvent(e), button(button) {}

	inline int getButton() const { return button; }

	std::string toString() const override {
		std::stringstream ss;
		ss << "MouseButtonReleasedEvent: " << button;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseButtonReleased)

  private:
	int button;
};

} // namespace CitronCore
