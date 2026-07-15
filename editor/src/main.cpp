#include "SDL3/SDL_keycode.h"
#include "editor.hpp"
#include <input.hpp>
#include <window.hpp>

using namespace CitronCore;
using namespace CitronInput;

int main() {
  Editor editor = Editor();
  editor.init();
  while (editor.isRunning()) {
    editor.update();
    editor.close();
  }
  return 0;
}
