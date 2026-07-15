#include "app.hpp"
#include <layer.hpp>
#include <window.hpp>

class EditorLayer : public CitronCore::Layer {
public:
  EditorLayer() : CitronCore::Layer("EditorLayer") {}

  void onAttach() override;
  void onDetach() override;
  void onUpdate() override;
  void onEvent(CitronCore::Event &e) override;
};

class Editor : public CitronCore::App {
public:
  Editor();
};
