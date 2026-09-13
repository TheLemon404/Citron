#include "inspector_panel.hpp"
#include "gui.hpp"
#include "editor.hpp"
#include "keyboard.hpp"

#include <imgui_stdlib.h>

bool InspectorPanel::collapsingHeader(const char *label,
									  const char *icon_open,
									  const char *icon_closed) {
	ImGuiWindow *window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext &g = *GImGui;
	const ImGuiStyle &style = g.Style;

	ImGuiID id = ImGui::GetID(label);
	ImGuiStorage *storage = ImGui::GetStateStorage();

	bool open = storage->GetBool(id, true);

	// Align button text to the left
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));

	// Format full-width label with your custom trailing/leading icons
	char buf[128];
	ImGui::SetWindowFontScale(4.0f);
	snprintf(buf, sizeof(buf), "%s    %s", (open ? icon_open : icon_closed),
			 label);
	ImGui::SetWindowFontScale(1.0f);

	// Draw full-width style frame
	if (ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f))) {
		open = !open;
		storage->SetBool(id, open);
	}

	ImGui::PopStyleVar();
	return open;
}

void InspectorPanel::drawComponentIcon(const std::string &name, WGPUTextureView iconView, ImVec2 iconMin, ImVec2 iconMax) {
	EditorIcons &icons = Editor::get().getLayer<GuiLayer>()->editorIcons;
	if (name == "Transform Component") {
		ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, iconMin, iconMax, icons.getIcon("Local").uv.Min, icons.getIcon("Local").uv.Max);
	} else if (name == "Mesh Component") {
		ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, iconMin, iconMax, icons.getIcon("Mesh").uv.Min, icons.getIcon("Mesh").uv.Max);
	} else if (name == "Perspective Camera Component") {
		ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, iconMin, iconMax, icons.getIcon("Camera").uv.Min, icons.getIcon("Camera").uv.Max);
	} else if (name == "Rigidbody Component") {
		ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, iconMin, iconMax, icons.getIcon("Rigidbody").uv.Min, icons.getIcon("Rigidbody").uv.Max);
	} else {
		ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, iconMin, iconMax, icons.getIcon("Component").uv.Min, icons.getIcon("Component").uv.Max);
	}
}

void InspectorPanel::onAttach() {}
void InspectorPanel::onDetach() {}
void InspectorPanel::onUpdate() {}

void InspectorPanel::onDraw() {
	EditorContext &context = Editor::get().getEditorContext();
	std::shared_ptr<Scene> currentScene = appContext.sceneManager.getActiveScene();

	EditorIcons &icons = Editor::get().getLayer<GuiLayer>()->editorIcons;
	WGPUTextureView iconView = icons.getTextureView();

	ImGui::Begin("Inspector");
	auto &registry = appContext.sceneManager.getActiveScene()->getRegistry();
	const std::variant<entt::entity, std::shared_ptr<System>> &selectedItem = context.getCurrentlySelectedItem();
	if (selectedItem.index() == 1) {
		std::shared_ptr<System> selectedSystem = std::get<std::shared_ptr<System>>(selectedItem);
		for (const auto &[hash, metadata] : ECSRegistry::getSystemRegistry()) {
			if (metadata.has(currentScene)) {
				std::shared_ptr<System> system = metadata.get(currentScene);
				if (collapsingHeader(metadata.name.c_str())) {
					// icon when opened
					ImVec2 rectMin = ImGui::GetItemRectMin();
					ImVec2 rectMax = ImGui::GetItemRectMax();
					float size = rectMax.y - rectMin.y;
					ImVec2 iconMin = ImVec2(rectMin.x, rectMin.y);
					ImVec2 iconMax = ImVec2(iconMin.x + size, iconMin.y + size);
					ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, iconMin, iconMax, icons.getIcon("System").uv.Min, icons.getIcon("System").uv.Max);

					if (ImGui::BeginTable("##ComponentMemberTable", 2,
										  ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInner)) {
						// First column gets a fixed width of 150 units
						ImGui::TableSetupColumn("##Fixed Col", ImGuiTableColumnFlags_WidthFixed, 115.0f);
						// Second column stretches to consume all remaining space in the row
						ImGui::TableSetupColumn("##Stretch Col", ImGuiTableColumnFlags_WidthStretch);

						for (const auto &member : metadata.members) {
							if (member.hideInEditor)
								continue;

							ImGui::TableNextColumn();
							ImGui::Text("%s", member.fieldName.c_str());
							ImGui::TableNextColumn();
							PropertyGuiDrawer drawer = member.drawer;
							if (drawer)
								drawer(member, system.get(), appContext.assetManager);
							else
								ImGui::Text("Drawing method undefined");
						}

						if (ImGui::BeginPopupContextWindow()) {
							if (ImGui::MenuItem("Remove System")) {
								metadata.remove(currentScene);
							}
							ImGui::EndPopup();
						}

						ImGui::EndTable();
					}
				} else {
					// icon when closed
					ImVec2 rectMin = ImGui::GetItemRectMin();
					ImVec2 rectMax = ImGui::GetItemRectMax();
					float size = rectMax.y - rectMin.y;
					ImVec2 iconMin = ImVec2(rectMin.x, rectMin.y);
					ImVec2 iconMax = ImVec2(iconMin.x + size, iconMin.y + size);
					ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, iconMin, iconMax, icons.getIcon("System").uv.Min, icons.getIcon("System").uv.Max);
				}
			}
		}
	}

	if (selectedItem.index() == 0 && registry.valid(std::get<entt::entity>(selectedItem))) {
		entt::entity selectedEntity = std::get<entt::entity>(selectedItem);
		for (const auto &[hash, metadata] : ECSRegistry::getComponentRegistry()) {
			if (metadata.has(registry, selectedEntity)) {
				void *component = metadata.get(registry, selectedEntity);
				if (collapsingHeader(metadata.name.c_str())) {
					// icon when open
					ImVec2 rectMin = ImGui::GetItemRectMin();
					ImVec2 rectMax = ImGui::GetItemRectMax();
					float size = rectMax.y - rectMin.y;
					ImVec2 iconMin = ImVec2(rectMin.x, rectMin.y);
					ImVec2 iconMax = ImVec2(iconMin.x + size, iconMin.y + size);
					// builting component icons based on name
					drawComponentIcon(metadata.name, iconView, iconMin, iconMax);

					if (ImGui::BeginPopupContextItem()) {
						if (ImGui::MenuItem("Remove Component")) {
							metadata.remove(registry, selectedEntity);
						}
						ImGui::EndPopup();
					}

					if (ImGui::BeginTable("##ComponentMemberTable", 2,
										  ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInner)) {

						// First column gets a fixed width of 150 units
						ImGui::TableSetupColumn("##Fixed Col", ImGuiTableColumnFlags_WidthFixed, 115.0f);
						// Second column stretches to consume all remaining space in the row
						ImGui::TableSetupColumn("##Stretch Col", ImGuiTableColumnFlags_WidthStretch);

						for (const auto &member : metadata.members) {
							if (member.hideInEditor)
								continue;

							ImGui::TableNextColumn();
							ImGui::Text("%s", member.fieldName.c_str());
							ImGui::TableNextColumn();
							PropertyGuiDrawer drawer = member.drawer;
							if (drawer)
								drawer(member, component, appContext.assetManager);
							else
								ImGui::Text("Drawing method undefined");
						}

						ImGui::EndTable();
					}
				} else {
					// icon when closed
					ImVec2 rectMin = ImGui::GetItemRectMin();
					ImVec2 rectMax = ImGui::GetItemRectMax();
					float size = rectMax.y - rectMin.y;
					ImVec2 iconMin = ImVec2(rectMin.x, rectMin.y);
					ImVec2 iconMax = ImVec2(iconMin.x + size, iconMin.y + size);
					drawComponentIcon(metadata.name, iconView, iconMin, iconMax);
				}
			}
		}

		if (ImGui::Button("Add Component", ImVec2(-FLT_MIN, 0.0f))) {
			ImGui::OpenPopup("ActionsPopup");
		}
		if (ImGui::BeginPopup("ActionsPopup")) {
			std::string componentSearchResult;
			ImGui::InputTextWithHint("##ComponentSearch",
									 "Enter Component Class Name",
									 &componentSearchResult, ImGuiInputTextFlags_EnterReturnsTrue);
			for (const auto &[id, component] : ECSRegistry::getComponentRegistry()) {
				std::string searchLower = componentSearchResult;
				std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

				if (component.name.starts_with(componentSearchResult)) {
					std::string componentIDName = "##" + component.name;
					if (ImGui::Selectable(componentIDName.c_str())) {
						componentSearchResult = component.name;
						component.add(registry, selectedEntity);
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					ImVec2 rectMin = ImGui::GetItemRectMin();
					ImVec2 rectMax = ImGui::GetItemRectMax();
					float delta = rectMax.y - rectMin.y;
					ImVec2 iconMax = ImVec2(rectMin.x + delta, rectMin.y + delta);
					ImGui::Dummy(ImVec2(0, delta));
					drawComponentIcon(component.name, iconView, rectMin, iconMax);
					ImGui::SameLine();
					ImGui::Text("%s", component.name.c_str());
				}
			}
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}

void InspectorPanel::onEvent(Event &e) {
	if (e.getCategoryFlags() & EventCategory::EventCategoryKeyboard) {
		if (e.getEventType() == EventType::KeyPressed) {
			KeyPressedEvent &event = (KeyPressedEvent &)e;
			if (event.getKeycode() == SDLK_B && event.getMods() & SDLK_LCTRL) {
				Editor::get().getEditorContext().setCurrentlySelectedItem(nullptr);
			}
		}
	}
}
