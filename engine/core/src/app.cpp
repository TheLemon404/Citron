#include "app.hpp"
#include "core.hpp"
#include "event.hpp"
#include "logger.hpp"

#include <functional>

using namespace CitronCore;

App *App::instance = nullptr;

App::App()
    : window("Citron Editor", 800, 600, CITRON_BIND_EVENT_FN(App::onEvent)) {
  CITRON_CORE_ASSERT(!instance, "App already exists");
  instance = this;
}

App::~App() {}

void App::init() {
  Logger::init();
  CITRON_CORE_INFO("Core logger initialized");
  CITRON_CLIENT_INFO("Client logger initialized");

  window.init();
  window.open();
}

void App::update() {
  while (running) {
    for (Layer *layer : layerStack) {
      layer->onUpdate();
    }

    window.pollEvents();
    window.swapBuffers();
  }
}

void App::close() {
  running = false;
  window.close();
}

void App::onEvent(Event &e) {
  EventDispatcher dispatcher(e);
  dispatcher.dispatch<WindowCloseEvent>(
      CITRON_BIND_EVENT_FN(App::onWindowClose));

  for (auto it = layerStack.end(); it != layerStack.begin();) {
    (*--it)->onEvent(e);
    if (e.handled)
      break;
  }
}

void App::pushLayer(Layer *layer) {
  layerStack.pushLayer(layer);
  layer->onAttach();
}

void App::popLayer(Layer *layer) {
  layerStack.popLayer(layer);
  layer->onDetach();
}

bool App::onWindowClose(Event &e) {
  running = false;
  return true;
}
