#include "editor.hpp"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_wgpu.h"
#include "graphics.hpp"
#include "logger.hpp"

#include <imgui.h>

void EditorLayer::onAttach() {}

void EditorLayer::onUpdate() {}

void EditorLayer::onDetach() {}

void EditorLayer::onEvent(CitronCore::Event &e) {}

Editor::Editor() : CitronCore::App() { pushLayer<EditorLayer>(); }
