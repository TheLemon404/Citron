#include "panel.hpp"

#include "editor.hpp"
#include <event.hpp>
#include <logger.hpp>

#include "imgui.h"

using namespace CitronCore;

void ConsolePanel::onAttach() {}
void ConsolePanel::onDetach() {}
void ConsolePanel::onUpdate() {}
void ConsolePanel::onDraw() {
	ImGui::Begin("Log");
	for (LogEntry &logEntry : Editor::get().getLogSink()->entries) {
		ImGui::TextUnformatted(logEntry.message.c_str());
	}
	ImGui::End();
}
void ConsolePanel::onEvent(Event &e) {}
