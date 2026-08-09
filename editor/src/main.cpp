#include "SDL3/SDL_keycode.h"
#include "editor.hpp"
#include <exception>
#include <input.hpp>
#include <window.hpp>
#include <instrumentor.hpp>

#include <core.hpp>

using namespace CitronCore;
using namespace CitronInput;

int main(int argc, char *argv[]) {
	// CITRON_PROFILE_BEGIN_SESSION("Citron", "citron_profile.json");
	std::string projectFilePath = argv[1];
	Editor editor = Editor(projectFilePath);
	editor.init();
	while (editor.isRunning()) {
		editor.update();
		editor.close();
	}
	// CITRON_PROFILE_END_SESSION();
	return 0;
}
