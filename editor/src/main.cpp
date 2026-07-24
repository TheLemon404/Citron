#include "SDL3/SDL_keycode.h"
#include "editor.hpp"
#include <input.hpp>
#include <window.hpp>

#include <core.hpp>

using namespace CitronCore;
using namespace CitronInput;

int main(int argc, char *argv[]) {

	if (argc != 2) {
		std::fprintf(stderr, "Usage: %s <project_path>\n", argv[0]);
		return 1;
	}

	try {
		std::string projectFilePath = argv[1];
		Editor editor = Editor(projectFilePath);
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
