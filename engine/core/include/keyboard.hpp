#pragma once

#include "citron_exports.hpp"

#include "event.hpp"
#include <sstream>

namespace CitronCore {
class CITRON_CORE_API KeyEvent : public Event {
  public:
	inline int getKeycode() const { return keycode; }
	EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

  protected:
	KeyEvent(SDL_Event *e, int keycode) : Event(e), keycode(keycode) {}
	int keycode;
};

class CITRON_CORE_API KeyTypedEvent : public KeyEvent {
  public:
	KeyTypedEvent(SDL_Event *e, const char *text)
		: KeyEvent(e, e->key.key), text(text) {}

	std::string toString() const override {
		std::stringstream ss;
		ss << "KeyTypedEvent: " << keycode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyTyped);

  private:
	const char *text;
};

class CITRON_CORE_API KeyPressedEvent : public KeyEvent {
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

class CITRON_CORE_API KeyJustPressedEvent : public KeyEvent {
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

class CITRON_CORE_API KeyReleasedEvent : public KeyEvent {
  public:
	KeyReleasedEvent(SDL_Event *e, int keycode) : KeyEvent(e, keycode) {}

	std::string toString() const override {
		std::stringstream ss;
		ss << "KeyReleasedEvent: " << keycode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyReleased);
};

class CITRON_CORE_API KeyJustReleasedEvent : public KeyEvent {
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
