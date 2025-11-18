#include "SandboxUI.hpp"
#include "SandboxPlayer.hpp"
#include "CellMatManager.hpp"
#include "Game/SandboxMap.hpp"
#include <algorithm>
#include "Game/RigidbodyManager.hpp"
#include "Box2DShapeBuilder.hpp"
#include "Game/RigidBodyObject.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <Engine/Core/VertexUtils.hpp>

std::unordered_map<CellMatType, CellMatUIInfo> SandBoxUI::s_materialUIInfo;

SandBoxUI::SandBoxUI()
{
	InitializeMaterialUIInfo();
}

SandBoxUI::~SandBoxUI()
{
}

void SandBoxUI::RenderMaterialBrushUI(SandboxPlayer* player)
{
	// Set window flags
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;

	// Material Selector Window
	if (m_showMaterialSelector) {
		ImGui::SetNextWindowSize(ImVec2(280, 50), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);

		ImGuiWindowFlags window_flags_selector = ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoTitleBar;// |
			//ImGuiWindowFlags_NoBackground; // | ImGuiWindowFlags_NoResize;

		if (ImGui::Begin("Material Brush", &m_showMaterialSelector, window_flags_selector)) {
			RenderMaterialSelector(player);
		}
		ImGui::End();
	}

	// Material Properties Window
	if (m_showMaterialProperties) {
		ImGui::SetNextWindowSize(ImVec2(350, 500), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(1250, 20), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Material Properties", &m_showMaterialProperties, window_flags)) {
			RenderMaterialProperties(player->GetSelectedMaterial());
		}
		ImGui::End();
	}
}

void SandBoxUI::RenderMaterialSelector(SandboxPlayer* player)
{
	CellMatType selectedMaterial = player->GetSelectedMaterial();

	// ==== brush set =========
	int brushSize = player->GetBrushSize();
	ImGui::Text("Brush Size: %d", brushSize);

	// Brush size slider
	int newBrushSize = brushSize;
	if (ImGui::SliderInt("##BrushSize", &newBrushSize, 1, 20, "%d")) {
		player->SetBrushSize(newBrushSize);
	}

	// === Current selected material display ===
	const CellMatUIInfo& currentInfo = s_materialUIInfo[selectedMaterial];
	ImGui::Text("Current: %s", currentInfo.m_name.c_str());
	ImGui::Separator();

	// Two-panel layout: Category buttons (left) + Material buttons (right)
	// Remove all table borders and make it borderless
	if (ImGui::BeginTable("MaterialSelector", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBody)) {
		ImGui::TableSetupColumn("C", ImGuiTableColumnFlags_WidthFixed, 80.0f);
		ImGui::TableSetupColumn("Materials", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		// === Left Panel: Category Selection ===
		//ImGui::Text("Type");
		ImGui::Separator();

		// Category buttons (vertical)
		auto renderCategoryButton = [&](const char* label, PhyType category, bool isToolsCategory = false) {
			bool isSelected = (m_selectedCategory == category && m_showToolsOnly == isToolsCategory);

			ImGui::PushID(label);

			// Set button colors based on state
			if (isSelected) {
				// Selected state - highlighted
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));        // Green
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f)); // Lighter green
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));  // Darker green
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));        // Bright green border
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
			}
			else {
				// Normal state - default gray
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));        // Gray
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Lighter gray
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));  // Darker gray
			}

			bool clicked = ImGui::Button(label, ImVec2(-1, 40));

			// Clean up styles
			if (isSelected) {
				ImGui::PopStyleVar();
				ImGui::PopStyleColor(4);
			}
			else {
				ImGui::PopStyleColor(3);
			}

			ImGui::PopID();
			return clicked;
			};

		// Category buttons with state management
		if (renderCategoryButton("Static\nSolid", PhyType::PHY_STATIC_SOLID)) {
			m_selectedCategory = PhyType::PHY_STATIC_SOLID;
			m_showToolsOnly = false;
		}

		if (renderCategoryButton("Move\nSolid", PhyType::PHY_MOVE_SOLID)) {
			m_selectedCategory = PhyType::PHY_MOVE_SOLID;
			m_showToolsOnly = false;
		}

		if (renderCategoryButton("Liquid", PhyType::PHY_LIQUID)) {
			m_selectedCategory = PhyType::PHY_LIQUID;
			m_showToolsOnly = false;
		}

		if (renderCategoryButton("Tools", PhyType::PHY_STATIC_SOLID, true)) {
			m_selectedCategory = PhyType::PHY_STATIC_SOLID;
			m_showToolsOnly = true;
		}

		ImGui::TableNextColumn();

		// === Right Panel: Material Selection ===
		//ImGui::Text("Materials");
		ImGui::Separator();

		// Get materials for selected category
		std::vector<CellMatType> materialsToShow;
		if (m_showToolsOnly) {
			materialsToShow = { CellMatType::MAT_EMPTY }; // Only tools
		}
		else {
			// Get materials by physics type
			for (const auto& pair : s_materialUIInfo) {
				if (pair.second.m_physType == m_selectedCategory && pair.first != CellMatType::MAT_EMPTY) {
					materialsToShow.push_back(pair.first);
				}
			}
		}

		// Render material buttons as squares in a grid layout
		float buttonSize = 35.0f;
		float spacing = 5.0f;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int buttonsPerRow = (int)((panelWidth + spacing) / (buttonSize + spacing));
		buttonsPerRow = std::max(1, buttonsPerRow);

		for (size_t i = 0; i < materialsToShow.size(); ++i) {
			CellMatType matType = materialsToShow[i];
			const CellMatUIInfo& info = s_materialUIInfo[matType];
			bool isSelected = (selectedMaterial == matType);

			// Start new row if needed
			if (i % buttonsPerRow != 0) {
				ImGui::SameLine();
			}

			ImGui::PushID((int)matType);

			// Set button colors
			ImGui::PushStyleColor(ImGuiCol_Button, info.m_color);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
				ImVec4(info.m_color.x * 1.3f, info.m_color.y * 1.3f, info.m_color.z * 1.3f, info.m_color.w));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive,
				ImVec4(info.m_color.x * 0.8f, info.m_color.y * 0.8f, info.m_color.z * 0.8f, info.m_color.w));

			// Highlight selected material
			if (isSelected) {
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 1.0f, 0.5f, 1.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);
			}

			// Square button with just color (no text)
	/*		if (ImGui::Button("", ImVec2(buttonSize, buttonSize))) {
				player->SetSelectedMaterial(matType);
			}*/

			bool singleClicked = ImGui::Button("", ImVec2(buttonSize, buttonSize));
			bool doubleClicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

			if (singleClicked && !doubleClicked) {
				player->SetSelectedMaterial(matType);
			}
			else if (doubleClicked) {
				player->SetSelectedMaterial(matType);
				m_showMaterialProperties = !m_showMaterialProperties;
			}

			// Clean up styles
			if (isSelected) {
				ImGui::PopStyleVar();
				ImGui::PopStyleColor();
			}
			ImGui::PopStyleColor(3);

			// Show material name on hover
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("%s", info.m_name.c_str());
				ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", info.m_description.c_str());
				ImGui::EndTooltip();
			}

			ImGui::PopID();
		}

		ImGui::EndTable();
	}


}

void SandBoxUI::RenderMaterialProperties(CellMatType selectedMaterial)
{
	const CellMatDef& matDef = CellMatManager::GetMaterialDef(selectedMaterial);
	const CellMatUIInfo& uiInfo = s_materialUIInfo[selectedMaterial];

	// Material header info
	ImGui::Text("Material: %s", uiInfo.m_name);
	ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", uiInfo.m_description);

	// Physics type label
	ImVec4 typeColor;
	switch (matDef.m_physicsType) {
	case PhyType::PHY_STATIC_SOLID:
		typeColor = ImVec4(0.8f, 0.6f, 0.4f, 1.0f);
		break;
	case PhyType::PHY_MOVE_SOLID:
		typeColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
		break;
	case PhyType::PHY_LIQUID:
		typeColor = ImVec4(0.3f, 0.6f, 1.0f, 1.0f);
		break;
	}

	ImGui::TextColored(typeColor, "Type: %s", uiInfo.m_physTypeName);
	ImGui::Separator();

	// Parameter table
	RenderParameterTable(matDef);
}

ImVec4 SandBoxUI::GetMaterialColor(CellMatType matType)
{
	auto it = s_materialUIInfo.find(matType);
	if (it != s_materialUIInfo.end()) {
		return it->second.m_color;
	}
	return ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Default gray
}

const char* SandBoxUI::GetPhysicsTypeName(PhyType physType)
{
	switch (physType) {
	case PhyType::PHY_STATIC_SOLID: return "Static Solid";
	case PhyType::PHY_MOVE_SOLID: return "Move Solid";
	case PhyType::PHY_LIQUID: return "Liquid";
	default: return "Unknown";
	}
}

const char* SandBoxUI::GetMaterialDescription(CellMatType matType)
{
	auto it = s_materialUIInfo.find(matType);
	if (it != s_materialUIInfo.end()) {
		return it->second.m_description.c_str();
	}
	return "Unknown Material";
}

void SandBoxUI::RenderRigidBodyPanel(SandboxPlayer* player)
{
	if (!m_showRigidBodyPanel) return;

	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(20, 600), ImGuiCond_FirstUseEver);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;

	if (ImGui::Begin("Rigid Body Creator", &m_showRigidBodyPanel, window_flags)) {

		// === Mode Toggle Button ===
		if (!m_rigidBodyDrawMode) {
			// 未进入模式 - 显示进入按钮
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));

			if (ImGui::Button("Start Drawing Rigid Body", ImVec2(-1, 50))) {
				m_rigidBodyDrawMode = true;
				player->ClearRigidBodyDrawnCells();
				player->SetSelectedMaterial(m_rigidBodyMaterial);
				m_showMaterialSelector = false;
			}

			ImGui::PopStyleColor(3);

			ImGui::Separator();
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
				"Click button to start drawing");
		}
		else {
			// 已进入模式 - 显示当前状态和控制按钮
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
				"RIGID BODY DRAW MODE ACTIVE");

			ImGui::Text("Cells Drawn: %d", player->GetRigidBodyDrawnCellsCount());

			ImGui::Separator();

			// === Material Selection for Rigid Body ===
			ImGui::Text("Draw Material:");

			// 显示几个常用材质供选择
			const std::vector<CellMatType> rbMaterials = {
				CellMatType::MAT_WOOD,
				CellMatType::MAT_STONE
			};

			float buttonSize = 40.0f;
			for (size_t i = 0; i < rbMaterials.size(); ++i) {
				if (i > 0) ImGui::SameLine();

				CellMatType matType = rbMaterials[i];
				const CellMatUIInfo& info = s_materialUIInfo[matType];
				bool isSelected = (m_rigidBodyMaterial == matType);

				ImGui::PushID((int)matType);

				// 设置按钮颜色
				ImGui::PushStyleColor(ImGuiCol_Button, info.m_color);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
					ImVec4(info.m_color.x * 1.3f, info.m_color.y * 1.3f,
						info.m_color.z * 1.3f, info.m_color.w));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive,
					ImVec4(info.m_color.x * 0.8f, info.m_color.y * 0.8f,
						info.m_color.z * 0.8f, info.m_color.w));

				if (isSelected) {
					ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);
				}

				if (ImGui::Button(info.m_name.c_str(), ImVec2(buttonSize * 1.5f, buttonSize))) {
					m_rigidBodyMaterial = matType;
					player->SetSelectedMaterial(matType);
				}

				if (isSelected) {
					ImGui::PopStyleVar();
					ImGui::PopStyleColor();
				}

				ImGui::PopStyleColor(3);
				ImGui::PopID();

				// Tooltip
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::Text("%s", info.m_name.c_str());
					ImGui::EndTooltip();
				}
			}

			ImGui::Separator();

			// === Action Buttons ===
			// Confirm Button - 生成刚体
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.5f, 0.9f, 1.0f));

			bool canConfirm = player->GetRigidBodyDrawnCellsCount() >= 3; // 至少需要3个cells
			if (!canConfirm) {
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
			}

			if (ImGui::Button("Confirm - Create Rigid Body", ImVec2(-1, 40))) {
				if (canConfirm) 
				{
					BaseMap* curMap = player->GetCurMap();
					dynamic_cast<SandboxMap*>(curMap)->GetRigidBodyManager()->CreateRigidBodies(player->GetRigidBodyDrawnCells(), b2_dynamicBody);

					m_rigidBodyDrawMode = false;
					m_showMaterialSelector = true;
					player->ClearRigidBodyDrawnCells();
				}
			}

			if (!canConfirm) {
				ImGui::PopStyleVar();
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::Text("Need at least 3 cells to create rigid body");
					ImGui::EndTooltip();
				}
			}

			ImGui::PopStyleColor(3);

			// Cancel Button - 取消退出
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.4f, 0.4f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));

			if (ImGui::Button("Cancel", ImVec2(-1, 30))) {
				m_rigidBodyDrawMode = false;
				player->ClearRigidBodyDrawnCells();
				// recover the material selector
				m_showMaterialSelector = true;
			}

			ImGui::PopStyleColor(3);

			ImGui::Separator();
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
				"Draw on canvas to create shape");
		}
	}
	ImGui::End();
}

void SandBoxUI::InitializeMaterialUIInfo()
{
//	// MAT_EMPTY
//	s_materialUIInfo[CellMatType::MAT_EMPTY].m_name = "Eraser";
//	s_materialUIInfo[CellMatType::MAT_EMPTY].m_description = "Tool - Remove material";
//	s_materialUIInfo[CellMatType::MAT_EMPTY].m_color = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
//	s_materialUIInfo[CellMatType::MAT_EMPTY].m_physType = PhyType::PHY_STATIC_SOLID;
//	s_materialUIInfo[CellMatType::MAT_EMPTY].m_physTypeName = "Tool";
//
//	// MAT_SAND
//	s_materialUIInfo[CellMatType::MAT_SAND].m_name = "Sand";
//	s_materialUIInfo[CellMatType::MAT_SAND].m_description = "Standard granular material";
//	s_materialUIInfo[CellMatType::MAT_SAND].m_color = ImVec4(0.96f, 0.64f, 0.38f, 1.0f);
//	s_materialUIInfo[CellMatType::MAT_SAND].m_physType = PhyType::PHY_MOVE_SOLID;
//	s_materialUIInfo[CellMatType::MAT_SAND].m_physTypeName = "Move Solid";
//
//	// MAT_SALT
//	s_materialUIInfo[CellMatType::MAT_SALT].m_name = "Salt";
//	s_materialUIInfo[CellMatType::MAT_SALT].m_description = "Fine granular material - flows easily";
//	s_materialUIInfo[CellMatType::MAT_SALT].m_color = ImVec4(0.97f, 0.97f, 1.0f, 1.0f);
//	s_materialUIInfo[CellMatType::MAT_SALT].m_physType = PhyType::PHY_MOVE_SOLID;
//	s_materialUIInfo[CellMatType::MAT_SALT].m_physTypeName = "Move Solid";
//
//	// MAT_WATER
//	s_materialUIInfo[CellMatType::MAT_WATER].m_name = "Water";
//	s_materialUIInfo[CellMatType::MAT_WATER].m_description = "Fluid material - liquid";
//	s_materialUIInfo[CellMatType::MAT_WATER].m_color = ImVec4(0.12f, 0.56f, 1.0f, 1.0f);
//	s_materialUIInfo[CellMatType::MAT_WATER].m_physType = PhyType::PHY_LIQUID;
//	s_materialUIInfo[CellMatType::MAT_WATER].m_physTypeName = "Liquid";
//
//	// MAT_STONE
//	s_materialUIInfo[CellMatType::MAT_STONE].m_name = "Stone";
//	s_materialUIInfo[CellMatType::MAT_STONE].m_description = "Static obstacle - hard solid";
//	s_materialUIInfo[CellMatType::MAT_STONE].m_color = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
//	s_materialUIInfo[CellMatType::MAT_STONE].m_physType = PhyType::PHY_STATIC_SOLID;
//	s_materialUIInfo[CellMatType::MAT_STONE].m_physTypeName = "Static Solid";
//
//	// MAT_WOOD
//	s_materialUIInfo[CellMatType::MAT_WOOD].m_name = "Wood";
//	s_materialUIInfo[CellMatType::MAT_WOOD].m_description = "Building material - light solid";
//	s_materialUIInfo[CellMatType::MAT_WOOD].m_color = ImVec4(0.55f, 0.27f, 0.07f, 1.0f);
//	s_materialUIInfo[CellMatType::MAT_WOOD].m_physType = PhyType::PHY_STATIC_SOLID;
//	s_materialUIInfo[CellMatType::MAT_WOOD].m_physTypeName = "Static Solid";


	for (const auto& pair : CellMatManager::GetAllMaterialDefs()) {
		CellMatType matType = pair.first;
		const CellMatDef& matDef = pair.second;

		s_materialUIInfo[matType].m_name = matDef.m_name;
		s_materialUIInfo[matType].m_description = matDef.m_description;
		s_materialUIInfo[matType].m_physType = matDef.m_physicsType;
		s_materialUIInfo[matType].m_physTypeName = GetPhysicsTypeName(matDef.m_physicsType);

		// 颜色转换
		Rgba8 m_color = matDef.m_color;
		s_materialUIInfo[matType].m_color = ImVec4(
			m_color.r / 255.0f, m_color.g / 255.0f, m_color.b / 255.0f, 1.0f
		);
	}
}

void SandBoxUI::RenderMaterialButton(CellMatType matType, bool isSelected, SandboxPlayer* player)
{
	const CellMatUIInfo& info = s_materialUIInfo[matType];

	ImGui::PushID((int)matType);

	// Set button style
	ImGui::PushStyleColor(ImGuiCol_Button, info.m_color);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
		ImVec4(info.m_color.x * 1.2f, info.m_color.y * 1.2f, info.m_color.z * 1.2f, info.m_color.w));

	if (isSelected) {
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 1.0f, 0.5f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
	}

	// Calculate button size
	float buttonWidth = (ImGui::GetContentRegionAvail().x - 5.0f) / 2.0f;

	if (ImGui::Button(info.m_name.c_str(), ImVec2(buttonWidth, 35))) {
		player->SetSelectedMaterial(matType);
	}

	if (isSelected) {
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}

	ImGui::PopStyleColor(2);

	// Show detailed info on hover
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::Text("%s", info.m_name);
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", info.m_description);
		ImGui::Text("Type: %s", info.m_physTypeName);
		ImGui::EndTooltip();
	}

	ImGui::PopID();
}

void SandBoxUI::RenderPhysicsGroup(const char* groupName, PhyType physType,
	const std::vector<CellMatType>& materials,
	SandboxPlayer* player)
{
	ImGui::Indent(10.0f);

	// Display 2 buttons per row
	int buttonsPerRow = 2;

	for (size_t i = 0; i < materials.size(); ++i) {
		CellMatType matType = materials[i];
		bool isSelected = (player->GetSelectedMaterial() == matType);

		if (i % buttonsPerRow != 0) {
			ImGui::SameLine();
		}

		RenderMaterialButton(matType, isSelected, player);
	}

	ImGui::Unindent(10.0f);
}

void SandBoxUI::RenderParameterTable(const CellMatDef& matDef)
{
	// Basic Physics Properties
	if (ImGui::CollapsingHeader("Basic Physics Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::BeginTable("BasicPhysics", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 150.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Density");
			ImGui::TableNextColumn(); ImGui::Text("%.2f", matDef.m_density);

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Friction");
			ImGui::TableNextColumn(); ImGui::Text("%.2f", matDef.m_friction);

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Restitution");
			ImGui::TableNextColumn(); ImGui::Text("%.2f", matDef.m_restitution);

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Viscosity");
			ImGui::TableNextColumn(); ImGui::Text("%.2f", matDef.m_viscosity);

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Gravity Multiplier");
			ImGui::TableNextColumn(); ImGui::Text("%.2f", matDef.m_gravityMultiplier);

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Terminal Velocity");
			ImGui::TableNextColumn(); ImGui::Text("%.1f", matDef.m_terminalVelocity);


			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Collision Momentum Transfer");
			ImGui::TableNextColumn(); ImGui::Text("%.3f", matDef.m_collisionMomentumTransfer);

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Activation Threshold");
			ImGui::TableNextColumn(); ImGui::Text("%.1f", matDef.m_activationThreshold);

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Neighbor Activation Chance");
			ImGui::TableNextColumn(); ImGui::Text("%.0f%%", matDef.m_neighborActivationChance * 100);

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Can Activate Neighbors");
			ImGui::TableNextColumn(); ImGui::Text("%s", matDef.m_canActivateNeighbors ? "Yes" : "No");

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Air Resistance");
			ImGui::TableNextColumn(); ImGui::Text("%.3f", matDef.m_airResistance);

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Collision Damping");
			ImGui::TableNextColumn(); ImGui::Text("%.2f", matDef.m_collisionDamping);

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Horizontal Damping");
			ImGui::TableNextColumn(); ImGui::Text("%.2f", matDef.m_horizontalDamping);

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Vertical Damping");
			ImGui::TableNextColumn(); ImGui::Text("%.2f", matDef.m_verticalDamping);

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Momentum Preservation");
			ImGui::TableNextColumn(); ImGui::Text("%.2f", matDef.m_momentumPreservation);

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Random Direction Chance");
			ImGui::TableNextColumn(); ImGui::Text("%.0f%%", matDef.m_randomDirectionChance * 100);

			ImGui::EndTable();
		}
	}

	// Move Solid Properties
	if (matDef.m_physicsType == PhyType::PHY_MOVE_SOLID) {
		if (ImGui::CollapsingHeader("Move Solid Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::BeginTable("MoveSolid", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 150.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				const auto& params = matDef.m_moveSolid;

				//ImGui::TableNextRow();
				//ImGui::TableNextColumn(); ImGui::Text("Slide Angle");
				//ImGui::TableNextColumn(); ImGui::Text("%.0f°", params.m_slideAngle);

				//ImGui::TableNextRow();
				//ImGui::TableNextColumn(); ImGui::Text("Collision Momentum Transfer");
				//ImGui::TableNextColumn(); ImGui::Text("%.3f", params.m_collisionMomentumTransfer);

				//ImGui::TableNextRow();
				//ImGui::TableNextColumn(); ImGui::Text("Activation Threshold");
				//ImGui::TableNextColumn(); ImGui::Text("%.1f", params.m_activationThreshold);

				//ImGui::TableNextRow();
				//ImGui::TableNextColumn(); ImGui::Text("Neighbor Activation Chance");
				//ImGui::TableNextColumn(); ImGui::Text("%.0f%%", params.m_neighborActivationChance * 100);

				//ImGui::TableNextRow();
				//ImGui::TableNextColumn(); ImGui::Text("Can Activate Neighbors");
				//ImGui::TableNextColumn(); ImGui::Text("%s", params.m_canActivateNeighbors ? "Yes" : "No");

				ImGui::EndTable();
			}
		}

		// Movement Damping
		if (ImGui::CollapsingHeader("Movement Damping")) {
			if (ImGui::BeginTable("MovementDamping", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 150.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				const auto& params = matDef.m_moveSolid;

				//ImGui::TableNextRow();
				//ImGui::TableNextColumn(); ImGui::Text("Air Resistance");
				//ImGui::TableNextColumn(); ImGui::Text("%.3f", params.m_airResistance);

				//ImGui::TableNextRow();
				//ImGui::TableNextColumn(); ImGui::Text("Collision Damping");
				//ImGui::TableNextColumn(); ImGui::Text("%.2f", params.m_collisionDamping);

				//ImGui::TableNextRow();
				//ImGui::TableNextColumn(); ImGui::Text("Horizontal Damping");
				//ImGui::TableNextColumn(); ImGui::Text("%.2f", params.m_horizontalDamping);

				//ImGui::TableNextRow();
				//ImGui::TableNextColumn(); ImGui::Text("Vertical Damping");
				//ImGui::TableNextColumn(); ImGui::Text("%.2f", params.m_verticalDamping);

				//ImGui::TableNextRow();
				//ImGui::TableNextColumn(); ImGui::Text("Momentum Preservation");
				//ImGui::TableNextColumn(); ImGui::Text("%.2f", params.m_momentumPreservation);

				//ImGui::TableNextRow();
				//ImGui::TableNextColumn(); ImGui::Text("Random Direction Chance");
				//ImGui::TableNextColumn(); ImGui::Text("%.0f%%", params.m_randomDirectionChance * 100);

				ImGui::EndTable();
			}
		}
	}

	// Liquid Properties
	if (matDef.m_physicsType == PhyType::PHY_LIQUID) {
		if (ImGui::CollapsingHeader("Liquid Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::BeginTable("Liquid", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 150.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				const auto& params = matDef.m_liquid;

				ImGui::TableNextRow();
				ImGui::TableNextColumn(); ImGui::Text("Flow Rate");
				ImGui::TableNextColumn(); ImGui::Text("%.2f", params.m_flowRate);

				ImGui::TableNextRow();
				ImGui::TableNextColumn(); ImGui::Text("Pressure Influence");
				ImGui::TableNextColumn(); ImGui::Text("%.2f", params.m_pressureInfluence);

				ImGui::TableNextRow();
				ImGui::TableNextColumn(); ImGui::Text("Surface Tension");
				ImGui::TableNextColumn(); ImGui::Text("%.2f", params.m_surfaceTension);

				ImGui::TableNextRow();
				ImGui::TableNextColumn(); ImGui::Text("Can Displace");
				ImGui::TableNextColumn(); ImGui::Text("%s", params.m_canDisplace ? "Yes" : "No");

				ImGui::TableNextRow();
				ImGui::TableNextColumn(); ImGui::Text("Displacement Force");
				ImGui::TableNextColumn(); ImGui::Text("%.2f", params.m_displacementForce);

				ImGui::EndTable();
			}
		}
	}

	// Interaction Properties
	if (ImGui::CollapsingHeader("Interaction Properties")) {
		if (ImGui::BeginTable("Interaction", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 150.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			const auto& params = matDef.m_interaction;

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Penetration Resistance");
			ImGui::TableNextColumn(); ImGui::Text("%.1f", params.m_penetrationResistance);

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Is Permeable");
			ImGui::TableNextColumn(); ImGui::Text("%s", params.m_isPermeable ? "Yes" : "No");

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("Is Soluble");
			ImGui::TableNextColumn(); ImGui::Text("%s", params.m_isSoluble ? "Yes" : "No");

			if (params.m_dissolutionRate > 0.0f) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn(); ImGui::Text("Dissolution Rate");
				ImGui::TableNextColumn(); ImGui::Text("%.3f", params.m_dissolutionRate);
			}

			ImGui::EndTable();
		}
	}
}