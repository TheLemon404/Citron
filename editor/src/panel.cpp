#include "panel.hpp"

#include "editor.hpp"
#include <event.hpp>
#include <logger.hpp>

#include "entt/entity/fwd.hpp"
#include "imgui.h"
#include "spdlog/common.h"
#include <ecs.hpp>
#include <imgui_stdlib.h>

using namespace CitronCore;
using namespace CitronECS;

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

void OutlinerPanel::onAttach() {}
void OutlinerPanel::onDetach() {}
void OutlinerPanel::onUpdate() {}
void OutlinerPanel::onDraw() {
	ImGui::Begin("Outliner");
	char text[64];
	ImGui::InputTextWithHint("Search", "Search by entity name", text, 64);

	if (ImGui::BeginTable("LogTable", 2,
						  ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed,
								75.0f);
		ImGui::TableHeadersRow();
		int i = 0;
		EditorContext &context = Editor::get()
									 .getLayerStack()
									 .getLayer<EditorLayer>()
									 ->getEditorContext();
		std::shared_ptr<Scene> currentEditedScene = context.getCurrentScene();
		if (currentEditedScene) {
			for (std::shared_ptr<System> &system :
				 currentEditedScene->getSystems()) {
				ImGui::PushID(i++);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Selectable(system->getName().c_str(), false,
								  ImGuiSelectableFlags_SpanAllColumns);
				ImGui::TableNextColumn();
				ImGui::Text("System");
				ImGui::PopID();
			}
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("--");
		ImGui::TableNextColumn();
		ImGui::Text("--");

		if (currentEditedScene) {
			const auto &view =
				currentEditedScene->getRegistry().view<EntityBase>();
			for (const entt::entity &entity : view) {
				auto &entityBase = view.get<EntityBase>(entity);
				ImGui::PushID(i++);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				if (ImGui::Selectable(entityBase.name.c_str(), false,
									  ImGuiSelectableFlags_SpanAllColumns)) {
					context.setCurrentSelectedEntity(&entity);
				}
				ImGui::TableNextColumn();
				ImGui::Text("System");
				ImGui::PopID();
			}

			if (ImGui::BeginPopupContextWindow()) {
				if (ImGui::MenuItem("Create Entity")) {
					currentEditedScene->createEntity();
				}

				ImGui::EndPopup();
			}
		}

		ImGui::EndTable();
	}

	ImGui::End();
}
void OutlinerPanel::onEvent(Event &e) {}

void InspectorPanel::onAttach() {}
void InspectorPanel::onDetach() {}
void InspectorPanel::onUpdate() {}
void InspectorPanel::onDraw() {
	ImGui::Begin("Inspector");
	EditorContext &context = Editor::get()
								 .getLayerStack()
								 .getLayer<EditorLayer>()
								 ->getEditorContext();
	if (const entt::entity *selectedEntity =
			context.getCurrentSelectedEntity()) {
		auto &registry = context.getCurrentScene()->getRegistry();
		EntityBase &entityBase = registry.get<EntityBase>(*selectedEntity);
		ImGui::InputText("Name", &entityBase.name);
	}
	ImGui::End();
}
void InspectorPanel::onEvent(Event &e) {}
