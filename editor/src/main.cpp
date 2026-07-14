#include <logger.hpp>
#include <window.hpp>

using namespace CitronIO;
using namespace CitronCore;

int main() {
  Logger::init();
  CITRON_CORE_INFO("Core logger initialized");
  CITRON_CLIENT_INFO("Client logger initialized");
  Window w = Window("Citron Editor", 800, 600);
  if (!w.init()) {
    return 1;
  }
  w.open();
  while (!w.shouldClose()) {
    w.pollEvents();
    w.swapBuffers();
  }
  w.close();
  return 0;
}
