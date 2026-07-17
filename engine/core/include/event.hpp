#pragma once

#include "SDL3/SDL_events.h"
#include "core.hpp"

#include <iostream>
#include <string>

namespace CitronCore {
enum class EventType {
	None = 0,
	WindowClose,
	WindowResize,
	WindowFocus,
	WindowLostFocus,
	WindowMoved,
	AppTick,
	AppUpdate,
	AppRender,
	KeyPressed,
	KeyJustPressed,
	KeyReleased,
	KeyJustReleased,
	MouseButtonPressed,
	MouseButtonReleased,
	MouseMoved,
	MouseScrolled,
};

enum EventCategory : int {
	None = 0,
	EventCategoryApp = BIT(1),
	EventCategoryInput = BIT(2),
	EventCategoryKeyboard = BIT(3),
	EventCategoryMouse = BIT(4),
	EventCategoryMouseButton = BIT(5),
	EventCategoryWindow = BIT(6),
};

#define EVENT_CLASS_TYPE(type)                                                 \
	static EventType getStaticType() { return EventType::type; }               \
	virtual EventType getEventType() const override {                          \
		return getStaticType();                                                \
	}                                                                          \
	virtual const char *getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category)                                         \
	virtual int getCategoryFlags() const override {                            \
		return EventCategory::category;                                        \
	}

class Event {
	friend class EventDispatcher;
	SDL_Event *event;

  public:
	Event(SDL_Event *internalEvent) : event(internalEvent) {}
	virtual EventType getEventType() const = 0;
	virtual const char *getName() const = 0;
	virtual int getCategoryFlags() const = 0;
	virtual std::string toString() const { return getName(); }

	void *getInternalEvent() const { return event; }

	inline bool isInCategory(EventCategory category) {
		return getCategoryFlags() & category;
	}

	bool handled = false;
};

class EventDispatcher {
	Event &event;

  public:
	EventDispatcher(Event &event) : event(event) {};

	template <typename T, typename F> bool dispatch(F &&func) {
		if (event.getEventType() == T::getStaticType()) {
			event.handled |= func(static_cast<T &>(event));
			return true;
		}
		return false;
	}
};

inline std::ostream &operator<<(std::ostream &os, const Event &e) {
	return os << e.toString();
}

} // namespace CitronCore
