#include "panel.hpp"

#include "editor.hpp"
#include <event.hpp>
#include <logger.hpp>

#include "imgui.h"
#include "imgui_internal.h"
#include "spdlog/common.h"

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

	if (ImGui::BeginTable("LogTable", 4,
						  ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 1.0f);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed,
								75.0f);
		ImGui::TableSetupColumn("Timestamp", ImGuiTableColumnFlags_WidthFixed,
								100.0f);
		ImGui::TableSetupColumn("Message");
		ImGui::TableSetupScrollFreeze(3, 1);
		ImGui::TableHeadersRow();

		for (LogEntry &logEntry : Editor::get().getLogSink()->entries) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImVec4 color;
			switch (logEntry.logLevel) {
			case spdlog::level::debug:
				color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
				break;
			case spdlog::level::info:
				color = ImVec4(0.27843137254f, 0.44705882352f, 0.70196078431f,
							   1.0f);
				break;
			case spdlog::level::warn:
				color = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
				break;
			case spdlog::level::err:
				color = ImVec4(1.0f, 0.3f, 0.0f, 1.0f);
				break;
			case spdlog::level::critical:
				color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
				break;
			default:
				color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
				break;
			}
			ImU32 cell_color = ImGui::ColorConvertFloat4ToU32(color); // Red
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_color);
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
