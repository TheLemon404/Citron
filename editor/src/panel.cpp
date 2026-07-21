#include "panel.hpp"

#include "editor.hpp"
#include <event.hpp>
#include <io.hpp>
#include <logger.hpp>

#include "entt/entity/fwd.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "spdlog/common.h"
#include <ecs.hpp>
#include <imgui_stdlib.h>

using namespace CitronCore;
using namespace CitronECS;

void AssetPanel::onAttach() {
	EditorContext &context = Editor::get()
								 .getLayerStack()
								 .getLayer<EditorLayer>()
								 ->getEditorContext();
	currentDirectory = context.projectRootFolderPath;
	refreshDirectoryListings();
}
void AssetPanel::onDetach() {}
void AssetPanel::onUpdate() {}
void AssetPanel::onDraw() {
	bool pendingRefreshDirectory = false;

	ImGui::Begin("Assets");
	ImGui::BeginGroup();
	if (ImGui::Button("Refresh")) {
		refreshDirectoryListings();
	}
	ImGui::SameLine();
	if (ImGui::Button("^")) {
		if (!currentDirectory.empty()) {
			currentDirectory =
				CitronIO::IO::getParentDirectory(currentDirectory);
			refreshDirectoryListings();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("+"))
		zoomLevel += 50;
	ImGui::SameLine();
	if (ImGui::Button("-"))
		zoomLevel -= 50;
	zoomLevel = std::max(100, zoomLevel);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(-1.0f);
	ImGui::InputText("##currentDirectory", &currentDirectory);

	ImGui::EndGroup();

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(20.0f, 20.0f));

	ImGui::BeginChild("AssetList");
	ImGui::Dummy(ImVec2(0.0f, 4.0f));
	int i = 0;
	for (const auto &entry : directoryListings) {
		ImGui::Columns(ImGui::GetCurrentWindow()->Size.x / zoomLevel, nullptr,
					   false);
		ImGui::PushID(i++);
		if (entry.isDirectory) {
			if (ImGui::Button(entry.name.c_str(),
							  ImVec2(zoomLevel * 0.9f, zoomLevel))) {
				currentDirectory = entry.path;
				pendingRefreshDirectory = true;
			}
		} else
			ImGui::Button(entry.name.c_str(),
						  ImVec2(zoomLevel * 0.9f, zoomLevel));
		ImGui::PopID();
		ImGui::NextColumn();
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();

	ImGui::End();

	if (pendingRefreshDirectory)
		refreshDirectoryListings();
}
void AssetPanel::onEvent(Event &e) {}

void AssetPanel::refreshDirectoryListings() {
	CITRON_CLIENT_INFO("Refreshed directory listings for: {}",
					   currentDirectory);
	directoryListings.clear();
	for (std::string &entry :
		 CitronIO::IO::getDirectoryEntries(currentDirectory)) {
		AssetCard card = {};
		card.path = entry;
		card.name = entry.substr(entry.find_last_of("\\") + 1);
		card.isDirectory = CitronIO::IO::isDirectory(entry);
		directoryListings.push_back(card);
	}
}

void ConsolePanel::onAttach() {}
void ConsolePanel::onDetach() {}
void ConsolePanel::onUpdate() {}
void ConsolePanel::onDraw() {
	ImGui::Begin("Log");
	if (ImGui::Button("Clear")) {
		Editor::get().getLogSink()->entries.clear();
	}

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
	EditorContext &context = Editor::get()
								 .getLayerStack()
								 .getLayer<EditorLayer>()
								 ->getEditorContext();
	std::shared_ptr<Scene> currentEditedScene = context.getCurrentScene();

	ImGui::Begin("Outliner");

	char text[64];
	ImGui::InputTextWithHint("Search", "Search by entity name", text, 64);
	ImGui::SameLine();
	if (ImGui::Button("+")) {
		ImGui::OpenPopup("ActionsPopup");
	}
	if (ImGui::BeginPopup("ActionsPopup")) {
		if (ImGui::Button("Add System")) {
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("Create Entity")) {
			if (currentEditedScene) {
				currentEditedScene->createEntity();
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginTable("LogTable", 2,
						  ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed,
								75.0f);
		ImGui::TableHeadersRow();

		int i = 0;
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
				ImGui::Text("Entity");
				ImGui::PopID();
			}

			if (ImGui::BeginPopupContextWindow()) {
				if (ImGui::MenuItem("Add System")) {
				}
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

		if (ImGui::CollapsingHeader("Entity Base",
									ImGuiTreeNodeFlags_DefaultOpen)) {
			float width = ImGui::GetContentRegionAvail().x;

			ImGui::InputText("Name", &entityBase.name);
		}
	}
	ImGui::End();
}
void InspectorPanel::onEvent(Event &e) {}
