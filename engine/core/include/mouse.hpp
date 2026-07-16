#pragma once

#include "event.hpp"
#include <sstream>

namespace CitronCore {
class MouseEvent : public Event {
  public:
	EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
};

class MouseScrolledEvent : public MouseEvent {
  public:
	MouseScrolledEvent(double dx, double dy) : dx(dx), dy(dy) {}

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

class MouseMovedEvent : public MouseEvent {
  public:
	MouseMovedEvent(double dx, double dy) : dx(dx), dy(dy) {}

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

class MouseButtonEvent : public MouseEvent {
  public:
	EVENT_CLASS_CATEGORY(EventCategoryMouseButton | EventCategoryMouse |
						 EventCategoryInput)
};

class MouseButtonPressedEvent : public MouseButtonEvent {
  public:
	MouseButtonPressedEvent(int button) : button(button) {}

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

class MouseButtonReleasedEvent : public MouseButtonEvent {
  public:
	MouseButtonReleasedEvent(int button) : button(button) {}

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
