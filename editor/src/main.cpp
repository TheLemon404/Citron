#include "SDL3/SDL_keycode.h"
#include "editor.hpp"
#include <input.hpp>
#include <window.hpp>

#include <core.hpp>

using namespace CitronCore;
using namespace CitronInput;

int main(int argc, char *argv[]) {
	std::string projectFilePath = argv[1];
	Editor editor = Editor(projectFilePath);
	editor.init();
	while (editor.isRunning()) {
		editor.update();
		editor.close();
	}
	return 0;
}
