#include "SDL3/SDL_keycode.h"
#include "editor.hpp"
#include <input.hpp>
#include <window.hpp>

using namespace CitronCore;
using namespace CitronInput;

int main() {
	try {
		Editor editor = Editor();
		editor.init();
		while (editor.isRunning()) {
			editor.update();
			editor.close();
		}
		return 0;
	} catch (const std::exception &e) {
		std::fprintf(stderr, "Unhandled exception: %s\n", e.what());
		return 1;
	} catch (...) {
		std::fprintf(stderr, "Unknown unhandled exception\n");
		return 1;
	}
}
