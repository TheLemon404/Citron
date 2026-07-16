#pragma once

#include "event.hpp"
#include <sstream>

namespace CitronCore {
class KeyEvent : public Event {
  public:
	inline int getKeycode() const { return keycode; }
	EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

  protected:
	KeyEvent(int keycode) : keycode(keycode) {}
	int keycode;
};

class KeyPressedEvent : public KeyEvent {
  public:
	KeyPressedEvent(int keycode, int repeatCount)
		: KeyEvent(keycode), repeatCount(repeatCount) {}

	inline int getRepeatCount() const { return repeatCount; }

	std::string toString() const override {
		std::stringstream ss;
		ss << "KeyPressedEvent: " << keycode << " (" << repeatCount
		   << " repeats)";
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyPressed);

  private:
	int repeatCount;
};

class KeyJustPressedEvent : public KeyEvent {
  public:
	KeyJustPressedEvent(int keycode) : KeyEvent(keycode) {}

	std::string toString() const override {
		std::stringstream ss;
		ss << "KeyJustPressedEvent: " << keycode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyJustPressed);
};

class KeyReleasedEvent : public KeyEvent {
  public:
	KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

	std::string toString() const override {
		std::stringstream ss;
		ss << "KeyReleasedEvent: " << keycode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyReleased);
};

class KeyJustReleasedEvent : public KeyEvent {
  public:
	KeyJustReleasedEvent(int keycode) : KeyEvent(keycode) {}

	std::string toString() const override {
		std::stringstream ss;
		ss << "KeyJustReleasedEvent: " << keycode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyJustReleased);
};

} // namespace CitronCore
