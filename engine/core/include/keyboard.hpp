#pragma once

#include "event.hpp"
#include <sstream>

namespace CitronCore {
class KeyEvent : public Event {
  public:
	inline int getKeycode() const { return keycode; }
	EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

  protected:
	KeyEvent(SDL_Event *e, int keycode) : Event(e), keycode(keycode) {}
	int keycode;
};

class KeyPressedEvent : public KeyEvent {
  public:
	KeyPressedEvent(SDL_Event *e, int keycode, int repeatCount, int mods)
		: KeyEvent(e, keycode), repeatCount(repeatCount), mods(mods) {}

	inline int getRepeatCount() const { return repeatCount; }

	inline int getMods() const { return mods; }

	std::string toString() const override {
		std::stringstream ss;
		ss << "KeyPressedEvent: " << keycode << " (" << repeatCount
		   << " repeats)";
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyPressed);

  private:
	int mods;
	int repeatCount;
};

class KeyJustPressedEvent : public KeyEvent {
  public:
	KeyJustPressedEvent(SDL_Event *e, int keycode, int mods)
		: KeyEvent(e, keycode), mods(mods) {}

	inline int getMods() const { return mods; }

	std::string toString() const override {
		std::stringstream ss;
		ss << "KeyJustPressedEvent: " << keycode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyJustPressed);

  private:
	int mods;
};

class KeyReleasedEvent : public KeyEvent {
  public:
	KeyReleasedEvent(SDL_Event *e, int keycode) : KeyEvent(e, keycode) {}

	std::string toString() const override {
		std::stringstream ss;
		ss << "KeyReleasedEvent: " << keycode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyReleased);
};

class KeyJustReleasedEvent : public KeyEvent {
  public:
	KeyJustReleasedEvent(SDL_Event *e, int keycode) : KeyEvent(e, keycode) {}

	std::string toString() const override {
		std::stringstream ss;
		ss << "KeyJustReleasedEvent: " << keycode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyJustReleased);
};

} // namespace CitronCore
