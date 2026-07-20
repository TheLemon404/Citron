#include "panel.hpp"

#include "editor.hpp"
#include <event.hpp>
#include <logger.hpp>

#include "imgui.h"
#include "imgui_internal.h"

using namespace CitronCore;

void ConsolePanel::onAttach() {}
void ConsolePanel::onDetach() {}
void ConsolePanel::onUpdate() {}
void ConsolePanel::onDraw() {
	ImGui::Begin("Log");
	ImGui::BeginTabBar("Menu");
	if (ImGui::TabItemButton("Clear", ImGuiTabItemFlags_Leading)) {
		Editor::get().getLogSink()->entries.clear();
	}
	ImGui::EndTabBar();

	if (ImGui::BeginTable("LogTable", 3,
						  ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed,
								150.0f);
		ImGui::TableSetupColumn("Timestamp", ImGuiTableColumnFlags_WidthFixed,
								150.0f);
		ImGui::TableSetupColumn("Message");
		ImGui::TableSetupScrollFreeze(3, 1);
		ImGui::TableHeadersRow();

		for (LogEntry &logEntry : Editor::get().getLogSink()->entries) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("%s", logEntry.type.c_str());
			ImGui::TableNextColumn();
			ImGui::Text("%u", logEntry.timestamp);
			ImGui::TableNextColumn();
			ImGui::TextUnformatted(logEntry.message.c_str());
		}
		ImGui::EndTable();
	}
	ImGui::End();
}
void ConsolePanel::onEvent(Event &e) {}
