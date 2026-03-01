#include "SandboxUI.hpp"
#include "SandboxPlayer.hpp"
#include "CellMatManager.hpp"
#include "Game/SandboxMap.hpp"
#include "Game/CellChunk.hpp"
#include "Game/RigidbodyManager.hpp"
#include "Box2DShapeBuilder.hpp"
#include "Game/RigidBodyObject.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Window/Window.hpp"
#include <algorithm>

extern Window* g_theWindow;

// ============================================================
//  小工具
// ============================================================
ImVec4 SandBoxUI::BrightenColor(ImVec4 c, float f)
{
	return ImVec4(
		std::min(c.x * f, 1.0f),
		std::min(c.y * f, 1.0f),
		std::min(c.z * f, 1.0f),
		c.w);
}

ImVec4 SandBoxUI::DarkenColor(ImVec4 c, float f)
{
	return ImVec4(c.x * f, c.y * f, c.z * f, c.w);
}

// ============================================================
//  构造 / 析构
// ============================================================
SandBoxUI::SandBoxUI() {}
SandBoxUI::~SandBoxUI() {}

// ============================================================
//  旧接口保留（内部转发，外部调用不需修改）
// ============================================================
void SandBoxUI::RenderMaterialBrushUI(SandboxPlayer* /*player*/)
{
	// 已集成进 RenderAll，此函数保留空壳以兼容旧调用点
}

void SandBoxUI::RenderRigidBodyPanel(SandboxPlayer* /*player*/)
{
	// 已集成进 RenderAll，此函数保留空壳以兼容旧调用点
}

// ============================================================
//  主入口 — 每帧调用一次
// ============================================================
void SandBoxUI::RenderAll(SandboxPlayer* player, SandboxMap* map)
{
	//----------------------------------------------------------
	// 1. 左侧固定侧边栏
	//----------------------------------------------------------
	ImGuiIO& io = ImGui::GetIO();
	float screenH = io.DisplaySize.y;

	// 折叠/展开按钮（贴左边缘）
	float collapseW = 18.0f;
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(collapseW, screenH), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::Begin("##CollapseBar", nullptr,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings);
	{
		const char* arrow = m_sidebarCollapsed ? ">" : "<";
		ImGui::SetCursorPos(ImVec2(1, screenH * 0.5f - 30));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 0.85f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
		if (ImGui::Button(arrow, ImVec2(collapseW - 2, 60)))
			m_sidebarCollapsed = !m_sidebarCollapsed;
		ImGui::PopStyleColor(2);
	}
	ImGui::End();
	ImGui::PopStyleVar(2);

	if (!m_sidebarCollapsed)
	{
		const float resizeHandleW = 5.0f;
		const float sidebarMinW = 180.0f;
		const float sidebarMaxW = 500.0f;
		float sideX = collapseW;

		// ---- 右边缘拖拽 resize handle（透明热区）----
		ImGui::SetNextWindowPos(ImVec2(sideX + m_sidebarWidth - resizeHandleW, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(resizeHandleW * 2.0f, screenH), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::Begin("##SidebarResize", nullptr,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoSavedSettings);
		{
			ImGui::InvisibleButton("##ResizeBtn", ImVec2(resizeHandleW * 2.0f, screenH));
			if (ImGui::IsItemHovered())
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				m_sidebarWidth += io.MouseDelta.x;
				m_sidebarWidth = GetClamped(m_sidebarWidth, sidebarMinW, sidebarMaxW);
			}
		}
		ImGui::End();
		ImGui::PopStyleVar(2);

		// ---- 侧边栏主体 ----
		ImGui::SetNextWindowPos(ImVec2(sideX, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(m_sidebarWidth, screenH), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.90f);

		ImGuiWindowFlags sideFlags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoSavedSettings;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

		if (ImGui::Begin("##Sidebar", nullptr, sideFlags))
		{
			// ---- 顶部状态栏：FPS + 帧时长（始终可见）----
			float fps = io.Framerate;
			float ms = 1000.0f / fps;
			ImVec4 fpsColor = fps >= 55.0f ? ImVec4(0.3f, 1.0f, 0.3f, 1.0f)
				: fps >= 30.0f ? ImVec4(1.0f, 0.8f, 0.0f, 1.0f)
				: ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
			ImGui::TextColored(fpsColor, "%.1f FPS", fps);
			ImGui::SameLine();
			ImGui::TextDisabled("(%.2f ms)", ms);
			ImGui::Separator();
			ImGui::Spacing();

			// ---- Tab 选择条（图标式按钮）----
			const char* tabLabels[] = { "Brush", "Debug", "Stats", "Rigidbody" };
			const ImVec4 tabActive = ImVec4(0.20f, 0.55f, 0.85f, 1.0f);
			const ImVec4 tabNormal = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
			float tabW = (m_sidebarWidth - 12) / 4.0f;

			for (int i = 0; i < 4; ++i)
			{
				if (i > 0) ImGui::SameLine(0, 2);
				bool active = (m_activeTab == i);
				ImGui::PushStyleColor(ImGuiCol_Button, active ? tabActive : tabNormal);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
					active ? BrightenColor(tabActive, 1.2f) : ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
				if (i == 3 && m_rigidBodyDrawMode)
				{
					// RigidBody 模式激活时闪橙色
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.0f, 1.0f));
					if (ImGui::Button(tabLabels[i], ImVec2(tabW, 28))) m_activeTab = i;
					ImGui::PopStyleColor(4);
				}
				else
				{
					if (ImGui::Button(tabLabels[i], ImVec2(tabW, 28))) m_activeTab = i;
					ImGui::PopStyleColor(2);
				}
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// ---- 内容区（带滚动）----
			ImGui::BeginChild("##SideContent", ImVec2(0, 0), false,
				ImGuiWindowFlags_HorizontalScrollbar);

			switch (m_activeTab)
			{
			case 0: RenderTab_Brush(player);          break;
			case 1: RenderTab_Debug(map);             break;
			case 2: RenderTab_Stats(map);             break;
			case 3: RenderTab_RigidBody(player, map); break;
			}

			ImGui::EndChild();
		}
		ImGui::End();
		ImGui::PopStyleVar(2);
	}

	//----------------------------------------------------------
	// 2. Cell Inspector（右侧浮窗，可关闭）
	//----------------------------------------------------------
	RenderCellInspector(map);
}

// ============================================================
//  Tab 0 — Brush
// ============================================================
void SandBoxUI::RenderTab_Brush(SandboxPlayer* player)
{
	RenderBrushControls(player);
	ImGui::Spacing();
	RenderCategoryBar(player);
	ImGui::Spacing();
	RenderMaterialGrid(player);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// 可折叠的材质属性
	ImGui::SetNextItemOpen(m_propertiesOpen, ImGuiCond_Once);
	if (ImGui::CollapsingHeader("Material Properties"))
	{
		m_propertiesOpen = true;
		RenderMaterialProperties(player->GetSelectedMaterial());
	}
	else
	{
		m_propertiesOpen = false;
	}
}

// ------------------------------------------------------------
void SandBoxUI::RenderBrushControls(SandboxPlayer* player)
{
	// 当前材质预览色块 + 名称
	CellMatType sel = player->GetSelectedMaterial();
	const CellMatUIInfo& info = CellMatManager::s_materialUIInfo[sel];

	ImGui::ColorButton("##matPreview", info.m_color,
		ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder,
		ImVec2(18, 18));
	ImGui::SameLine();
	ImGui::TextUnformatted(info.m_name.c_str());

	// Brush Size
	int brushSize = player->GetBrushSize();
	ImGui::PushItemWidth(-1);
	if (ImGui::SliderInt("##BrushSize", &brushSize, 1, 30, "Size: %d"))
		player->SetBrushSize(brushSize);
	ImGui::PopItemWidth();
}

// ------------------------------------------------------------
void SandBoxUI::RenderCategoryBar(SandboxPlayer* player)
{
	struct CatInfo { const char* label; PhyType type; bool toolsOnly; };
	CatInfo cats[] = {
		{ "Static",  PhyType::PHY_STATIC_SOLID,      false },
		{ "Solid",   PhyType::PHY_MOVE_SOLID,         false },
		{ "Liquid",  PhyType::PHY_LIQUID,             false },
		{ "CA",      PhyType::PHY_CELLULAR_AUTOMATON, false },
		{ "Tools",   PhyType::PHY_STATIC_SOLID,       true  },
	};

	float btnW = (m_sidebarWidth - 12 - 4 * 3) / 5.0f; // 5 buttons, 4 gaps

	for (int i = 0; i < 5; ++i)
	{
		if (i > 0) ImGui::SameLine(0, 3);

		bool active = (m_selectedCategory == cats[i].type && m_showToolsOnly == cats[i].toolsOnly);
		ImGui::PushStyleColor(ImGuiCol_Button,
			active ? ImVec4(0.20f, 0.65f, 0.20f, 1.0f) : ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
			active ? ImVec4(0.30f, 0.75f, 0.30f, 1.0f) : ImVec4(0.40f, 0.40f, 0.40f, 1.0f));
		if (active)
		{
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 1.0f, 0.3f, 1.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
		}
		ImGui::PushID(i + 100);
		if (ImGui::Button(cats[i].label, ImVec2(btnW, 24)))
		{
			m_selectedCategory = cats[i].type;
			m_showToolsOnly = cats[i].toolsOnly;
		}
		ImGui::PopID();
		if (active) { ImGui::PopStyleVar(); ImGui::PopStyleColor(3); }
		else { ImGui::PopStyleColor(2); }
	}

	(void)player;
}

// ------------------------------------------------------------
void SandBoxUI::RenderMaterialGrid(SandboxPlayer* player)
{
	CellMatType selectedMaterial = player->GetSelectedMaterial();

	// 收集当前分类的材质
	std::vector<CellMatType> mats;
	for (const auto& pair : CellMatManager::s_materialUIInfo)
	{
		if (m_showToolsOnly)
		{
			if (pair.first == CellMatType::MAT_EMPTY)
				mats.push_back(pair.first);
		}
		else
		{
			if (pair.second.m_physType == m_selectedCategory &&
				pair.first != CellMatType::MAT_EMPTY)
				mats.push_back(pair.first);
		}
	}

	// 图标格子（38px × N列）
	const float iconSz = 38.0f;
	const float gap = 4.0f;
	float avail = ImGui::GetContentRegionAvail().x;
	int cols = std::max(1, (int)((avail + gap) / (iconSz + gap)));

	for (size_t i = 0; i < mats.size(); ++i)
	{
		if (i % cols != 0) ImGui::SameLine(0, (int)gap);

		CellMatType mt = mats[i];
		const CellMatUIInfo& info = CellMatManager::s_materialUIInfo[mt];
		bool isSel = (selectedMaterial == mt);

		ImGui::PushID((int)mt);
		ImGui::PushStyleColor(ImGuiCol_Button, info.m_color);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, BrightenColor(info.m_color, 1.35f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, DarkenColor(info.m_color, 0.80f));
		if (isSel)
		{
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 1.0f, 0.5f, 1.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.5f);
		}

		bool singleClick = ImGui::Button("", ImVec2(iconSz, iconSz));
		bool doubleClick = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

		if (singleClick && !doubleClick)
			player->SetSelectedMaterial(mt);

		if (isSel) { ImGui::PopStyleVar(); ImGui::PopStyleColor(); }
		ImGui::PopStyleColor(3);

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextUnformatted(info.m_name.c_str());
			if (!info.m_description.empty())
				ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", info.m_description.c_str());
			ImGui::EndTooltip();
		}

		ImGui::PopID();
	}
}

// ------------------------------------------------------------
void SandBoxUI::RenderMaterialProperties(CellMatType selected)
{
	const CellMatDef& matDef = CellMatManager::GetMaterialDef(selected);
	const CellMatUIInfo& uiInfo = CellMatManager::s_materialUIInfo[selected];

	// 名称 + 类型标签
	ImGui::TextUnformatted(uiInfo.m_name.c_str());
	ImGui::SameLine();
	ImVec4 typeColor;
	switch (matDef.m_physicsType) {
	case PhyType::PHY_STATIC_SOLID:      typeColor = ImVec4(0.80f, 0.60f, 0.40f, 1.0f); break;
	case PhyType::PHY_MOVE_SOLID:        typeColor = ImVec4(1.00f, 0.80f, 0.00f, 1.0f); break;
	case PhyType::PHY_LIQUID:            typeColor = ImVec4(0.30f, 0.60f, 1.00f, 1.0f); break;
	case PhyType::PHY_CELLULAR_AUTOMATON:typeColor = ImVec4(0.80f, 0.30f, 0.90f, 1.0f); break;
	default:                             typeColor = ImVec4(0.70f, 0.70f, 0.70f, 1.0f); break;
	}
	ImGui::TextColored(typeColor, "[%s]", uiInfo.m_physTypeName);

	if (!uiInfo.m_description.empty())
		ImGui::TextDisabled("%s", uiInfo.m_description.c_str());

	ImGui::Spacing();
	RenderParameterTable(matDef);
}

// ============================================================
//  Tab 1 — Debug
// ============================================================
void SandBoxUI::RenderTab_Debug(SandboxMap* map)
{
	RenderDebugGrid(map);
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	RenderDebugColorMode(map);
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	RenderDebugRigidBody(map);
}

// ------------------------------------------------------------
void SandBoxUI::RenderDebugGrid(SandboxMap* map)
{
	ImGui::SeparatorText("Chunk Grid");
	ImGui::Checkbox("Draw Grid Lines", &map->m_debugSettings.m_drawChunkGrid);
	ImGui::Checkbox("Highlight Static", &map->m_debugSettings.m_drawStaticChunks);
	ImGui::Checkbox("Highlight Dynamic", &map->m_debugSettings.m_drawDynamicChunks);
}

// ------------------------------------------------------------
void SandBoxUI::RenderDebugColorMode(SandboxMap* map)
{
	ImGui::SeparatorText("Cell Color Mode");

	auto radio = [&](const char* label, CellColorMode mode, const char* tip)
		{
			if (ImGui::RadioButton(label, map->m_debugSettings.m_colorMode == mode))
				map->m_debugSettings.m_colorMode = mode;
			if (tip && ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextUnformatted(tip);
				ImGui::EndTooltip();
			}
		};

	radio("Normal",
		CellColorMode::NORMAL,
		"Display cells with their original material colors");

	radio("Static / Dynamic",
		CellColorMode::STATIC_DYNAMIC,
		"Red=Static  Green=Dynamic  White=StaticSolid");

	radio("Movement Heatmap",
		CellColorMode::FRAME_COUNT,
		"Green=just moved  Red=settled 80+ frames  White=StaticSolid");

	// 图例
	if (map->m_debugSettings.m_colorMode != CellColorMode::NORMAL)
	{
		ImGui::Spacing();
		ImGui::TextDisabled("Legend:");
		if (map->m_debugSettings.m_colorMode == CellColorMode::STATIC_DYNAMIC)
		{
			ImGui::ColorButton("##r", ImVec4(1, 0, 0, 1), ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
			ImGui::SameLine(); ImGui::TextUnformatted("Static particles");
			ImGui::ColorButton("##g", ImVec4(0, 1, 0, 1), ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
			ImGui::SameLine(); ImGui::TextUnformatted("Dynamic particles");
			ImGui::ColorButton("##w", ImVec4(1, 1, 1, 1), ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
			ImGui::SameLine(); ImGui::TextUnformatted("Static solids");
		}
		else
		{
			ImGui::ColorButton("##g2", ImVec4(0, 1, 0, 1), ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
			ImGui::SameLine(); ImGui::TextUnformatted("0 frames (active)");
			ImGui::ColorButton("##y", ImVec4(1, 1, 0, 1), ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
			ImGui::SameLine(); ImGui::TextUnformatted("~40 frames");
			ImGui::ColorButton("##r2", ImVec4(1, 0, 0, 1), ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
			ImGui::SameLine(); ImGui::TextUnformatted("80+ frames (settled)");
		}
	}
}

// ------------------------------------------------------------
void SandBoxUI::RenderDebugRigidBody(SandboxMap* map)
{
	ImGui::SeparatorText("Rigid Body Debug");
	ImGui::Checkbox("Marching Squares", &map->m_debugSettings.m_drawMarchingSquares);
	ImGui::Checkbox("Douglas-Peucker", &map->m_debugSettings.m_drawDouglas);
	ImGui::Checkbox("Triangle Mesh", &map->m_debugSettings.m_drawTriangleMesh);
}

// ============================================================
//  Tab 2 — Stats
// ============================================================
void SandBoxUI::RenderTab_Stats(SandboxMap* map)
{
	ImGui::SeparatorText("Cell Statistics");

	// 从 map 公开的缓存字段读取（const 访问）
	// 若后续需要更多字段，只需在 SandboxMap 加 getter 即可
	ImGui::Text("Non-Empty Cells : %d", map->m_cachedNonEmptyCells);
	ImGui::Text("Sand            : %d", map->m_cachedSandCells);
	ImGui::Text("Water           : %d", map->m_cachedWaterCells);
	ImGui::Text("Stone           : %d", map->m_cachedStoneCells);
	ImGui::Spacing();
	ImGui::Text("Total Sets      : %d", map->m_totalMaterialsSet);

	ImGui::Spacing();
	ImGui::SeparatorText("Chunk Grid");
	ImGui::Text("Grid Size: %d x %d chunks",
		map->GetChunkGridSize().x, map->GetChunkGridSize().y);
	ImGui::Text("Map Size : %d x %d cells",
		map->GetMapSize().x, map->GetMapSize().y);

	// FPS is always shown in the sidebar header above the tabs
}

// ============================================================
//  Tab 3 — Rigid Body
// ============================================================
void SandBoxUI::RenderTab_RigidBody(SandboxPlayer* player, SandboxMap* map)
{
	ImGui::SeparatorText("Rigid Body Creator");

	if (!m_rigidBodyDrawMode)
	{
		// 入口按钮
		ImGui::TextDisabled("Draw a shape then confirm to create a physics body.");
		ImGui::Spacing();

		ImGui::TextUnformatted("Material:");
		ImGui::SameLine();
		const std::vector<CellMatType> rbMats = { CellMatType::MAT_WOOD, CellMatType::MAT_STONE };
		float btnW = 70.0f;
		for (size_t i = 0; i < rbMats.size(); ++i)
		{
			if (i > 0) ImGui::SameLine(0, 4);
			CellMatType mt = rbMats[i];
			bool isSel = (m_rigidBodyMaterial == mt);
			const CellMatUIInfo& info = CellMatManager::s_materialUIInfo[mt];

			ImGui::PushID((int)mt + 9000);
			ImGui::PushStyleColor(ImGuiCol_Button, info.m_color);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, BrightenColor(info.m_color, 1.3f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, DarkenColor(info.m_color, 0.8f));
			if (isSel)
			{
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
			}
			if (ImGui::Button(info.m_name.c_str(), ImVec2(btnW, 30)))
			{
				m_rigidBodyMaterial = mt;
				player->SetSelectedMaterial(mt);
			}
			if (isSel) { ImGui::PopStyleVar(); ImGui::PopStyleColor(); }
			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}

		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.70f, 0.20f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.80f, 0.30f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.60f, 0.10f, 1.0f));
		if (ImGui::Button("Start Drawing", ImVec2(-1, 44)))
		{
			m_rigidBodyDrawMode = true;
			player->ClearRigidBodyDrawnCells();
			player->SetSelectedMaterial(m_rigidBodyMaterial);
			m_activeTab = 3; // 保持在 RBody Tab
		}
		ImGui::PopStyleColor(3);
	}
	else
	{
		// 绘制模式激活中
		ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "  DRAW MODE ACTIVE");
		ImGui::Text("Cells Drawn: %d", player->GetRigidBodyDrawnCellsCount());
		ImGui::Spacing();
		ImGui::TextDisabled("Paint on the canvas, then confirm.");
		ImGui::Spacing();

		// Confirm
		bool canConfirm = player->GetRigidBodyDrawnCellsCount() >= 3;
		if (!canConfirm)
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.45f);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.55f, 0.90f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.65f, 1.00f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.45f, 0.80f, 1.0f));
		if (ImGui::Button("Confirm  (Create Body)", ImVec2(-1, 42)) && canConfirm)
		{
			BaseMap* curMap = player->GetCurMap();
			dynamic_cast<SandboxMap*>(curMap)->GetRigidBodyManager()
				->CreateRigidBodies(player->GetRigidBodyDrawnCells(), b2_dynamicBody);
			m_rigidBodyDrawMode = false;
			player->ClearRigidBodyDrawnCells();
		}
		ImGui::PopStyleColor(3);
		if (!canConfirm)
		{
			ImGui::PopStyleVar();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text("Need at least 3 cells");
				ImGui::EndTooltip();
			}
		}

		ImGui::Spacing();

		// Cancel
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.25f, 0.25f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.35f, 0.35f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.15f, 0.15f, 1.0f));
		if (ImGui::Button("Cancel", ImVec2(-1, 28)))
		{
			m_rigidBodyDrawMode = false;
			player->ClearRigidBodyDrawnCells();
		}
		ImGui::PopStyleColor(3);
	}

	(void)map;
}

// ============================================================
//  Cell Inspector（右侧浮窗）
// ============================================================
void SandBoxUI::RenderCellInspector(SandboxMap* map)
{
	// 小切换按钮（右上角）
	ImGuiIO& io = ImGui::GetIO();
	float btnX = io.DisplaySize.x - 110.0f;
	ImGui::SetNextWindowPos(ImVec2(btnX, 4), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(108, 24), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::Begin("##InspTgl", nullptr,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoSavedSettings);
	{
		ImGui::PushStyleColor(ImGuiCol_Button,
			m_showCellInspector ? ImVec4(0.20f, 0.55f, 0.85f, 1.0f)
			: ImVec4(0.25f, 0.25f, 0.25f, 0.80f));
		if (ImGui::Button("Cell Inspector", ImVec2(106, 20)))
			m_showCellInspector = !m_showCellInspector;
		ImGui::PopStyleColor();
	}
	ImGui::End();
	ImGui::PopStyleVar(2);

	if (!m_showCellInspector) return;

	float inspW = 260.0f;
	float inspX = io.DisplaySize.x - inspW - 2.0f;
	ImGui::SetNextWindowPos(ImVec2(inspX, 30), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(inspW, 420), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.88f);

	ImGuiWindowFlags inspFlags =
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::Begin("##CellInsp", nullptr, inspFlags))
	{
		ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Cell Inspector");
		ImGui::Separator();

		// 鼠标格子坐标
		int mx = map->m_mouseGridX;
		int my = map->m_mouseGridY;
		ImGui::Text("Grid  : (%d, %d)", mx, my);

		IntVec2 chunkIdx = CellChunk::WorldToChunkIndex(mx, my);
		IntVec2 localPos = CellChunk::WorldToLocal(mx, my);
		ImGui::Text("Chunk : (%d, %d)  Local: (%d, %d)",
			chunkIdx.x, chunkIdx.y, localPos.x, localPos.y);

		// World position via camera
		Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
		Vec2 worldPos = AABB2(
			map->GetCurPlayer()->m_camera.GetOrthoBottomLeft(),
			map->GetCurPlayer()->m_camera.GetOrthoTopRight()
		).GetPointAtUV(mouseUV);
		ImGui::Text("World : (%.1f, %.1f)", worldPos.x, worldPos.y);

		// Cell data
		if (!map->IsInBounds(mx, my))
		{
			ImGui::TextDisabled("(out of bounds)");
		}
		else
		{
			CellChunk* chunk = const_cast<SandboxMap*>(map)->GetChunkByWorldPos(mx, my);
			if (chunk)
			{
				const Cell& cell = chunk->GetLocalCell(localPos.x, localPos.y);

				ImGui::Separator();
				// --- Chunk section ---
				ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 1.0f), "Chunk");
				ImGui::Text("Phase : %d   Dirty: %s",
					chunk->GetPhaseIndex(),
					chunk->IsDirty() ? "Yes" : "No");
				ImGui::Text("Active cells: %d", chunk->GetActiveCellCount());

				ImGui::Separator();
				// --- Cell section ---
				const char* matName = CellMatManager::s_materialUIInfo.count(cell.m_type.load())
					? CellMatManager::s_materialUIInfo.at(cell.m_type.load()).m_name.c_str()
					: "?";
				ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 1.0f), "Cell");
				ImGui::Text("Material : %s", matName);

				// Color swatch
				ImVec4 swatchColor(
					cell.m_color.r / 255.0f,
					cell.m_color.g / 255.0f,
					cell.m_color.b / 255.0f,
					cell.m_color.a / 255.0f);
				ImGui::SameLine();
				ImGui::ColorButton("##cellColor", swatchColor,
					ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder,
					ImVec2(14, 14));

				// Physics
				ImGui::Separator();
				ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 1.0f), "Physics");

				// 速度进度条（可视化）
				float maxV = 200.0f;
				float vx = cell.m_velocityX;
				float vy = cell.m_velocityY;
				ImGui::Text("Vel X: %+7.2f", vx);
				ImGui::SameLine();
				ImGui::ProgressBar((vx + maxV) / (2.0f * maxV), ImVec2(-1, 8), "");

				ImGui::Text("Vel Y: %+7.2f", vy);
				ImGui::SameLine();
				ImGui::ProgressBar((vy + maxV) / (2.0f * maxV), ImVec2(-1, 8), "");

				ImGui::Text("Accum: (%.2f, %.2f)",
					cell.m_accumulMoveX, cell.m_accumulMoveY);

				// State
				ImGui::Separator();
				ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 1.0f), "State");
				ImGui::Text("Free fall    : %s", cell.m_isFreeFalling ? "Yes" : "No");
				ImGui::Text("Updated/frame: %s", cell.m_updatedThisFrame ? "Yes" : "No");
				ImGui::Text("Still frames : %d", cell.m_framesWithoutMovement);

				CellMatType ct = cell.m_type.load();
				if (ct == CellMatType::MAT_WATER ||
					ct == CellMatType::MAT_OIL ||
					ct == CellMatType::MAT_LAVA)
				{
					ImGui::Text("Recollide    : %d", cell.m_liquidReCollideTimes);
				}

				// Chemical
				if (cell.m_lifeCountDown >= 0 || cell.m_flameCountDown >= 0 ||
					cell.m_dissolveCountDown >= 0 || cell.m_corrosionCountDown >= 0)
				{
					ImGui::Separator();
					ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 1.0f), "Chemical");
					if (cell.m_lifeCountDown >= 0) ImGui::Text("Life      : %d", cell.m_lifeCountDown);
					if (cell.m_flameCountDown >= 0) ImGui::Text("Flame     : %d", cell.m_flameCountDown);
					if (cell.m_dissolveCountDown >= 0) ImGui::Text("Dissolve  : %d", cell.m_dissolveCountDown);
					if (cell.m_corrosionCountDown >= 0) ImGui::Text("Corrosion : %d", cell.m_corrosionCountDown);
				}
			}
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

// ============================================================
//  RenderParameterTable  —  材质参数只读展示（整理版）
// ============================================================
void SandBoxUI::RenderParameterTable(const CellMatDef& def)
{
	// ---- 基础物理 ----
	if (ImGui::CollapsingHeader("Basic Physics", ImGuiTreeNodeFlags_DefaultOpen))
	{
		constexpr ImGuiTableFlags tflags =
			ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp;

		if (ImGui::BeginTable("##bp", 2, tflags))
		{
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 140.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			auto row = [](const char* label, const char* fmt, auto val)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn(); ImGui::TextUnformatted(label);
					ImGui::TableNextColumn(); ImGui::Text(fmt, val);
				};

			row("Density", "%.2f", def.m_density);
			row("Friction", "%.2f", def.m_friction);
			row("Restitution", "%.2f", def.m_restitution);
			row("Viscosity", "%.2f", def.m_viscosity);
			row("Gravity Multiplier", "%.2f", def.m_gravityMultiplier);
			row("Terminal Velocity", "%.1f", def.m_terminalVelocity);

			ImGui::EndTable();
		}
	}

	// ---- 运动衰减 ----
	if (ImGui::CollapsingHeader("Motion Damping"))
	{
		if (ImGui::BeginTable("##md", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
		{
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 140.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			auto row = [](const char* label, const char* fmt, auto val)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn(); ImGui::TextUnformatted(label);
					ImGui::TableNextColumn(); ImGui::Text(fmt, val);
				};

			row("Air Resistance", "%.3f", def.m_airResistance);
			row("Collision Damping", "%.2f", def.m_collisionDamping);
			row("Horizontal Damping", "%.2f", def.m_horizontalDamping);
			row("Vertical Damping", "%.2f", def.m_verticalDamping);
			row("Momentum Preservation", "%.2f", def.m_momentumPreservation);
			row("Random Direction Chance", "%.0f%%", def.m_randomDirectionChance * 100.0f);
			row("Col Momentum Transfer", "%.3f", def.m_collisionMomentumTransfer);
			row("Activation Threshold", "%.2f", def.m_activationThreshold);
			row("Neighbor Activate Chance", "%.0f%%", def.m_neighborActivationChance * 100.0f);
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn(); ImGui::TextUnformatted("Can Activate Neighbors");
				ImGui::TableNextColumn(); ImGui::TextUnformatted(def.m_canActivateNeighbors ? "Yes" : "No");
			}

			ImGui::EndTable();
		}
	}

	// ---- 液体专属 ----
	if (def.m_physicsType == PhyType::PHY_LIQUID &&
		ImGui::CollapsingHeader("Liquid Params", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::BeginTable("##lq", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
		{
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 140.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			const auto& lp = def.m_liquid;
			ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted("Flow Rate");
			ImGui::TableNextColumn(); ImGui::Text("%.2f", lp.m_flowRate);
			ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted("Pressure Influence");
			ImGui::TableNextColumn(); ImGui::Text("%.2f", lp.m_pressureInfluence);
			ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted("Surface Tension");
			ImGui::TableNextColumn(); ImGui::Text("%.2f", lp.m_surfaceTension);
			ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted("Can Displace");
			ImGui::TableNextColumn(); ImGui::TextUnformatted(lp.m_canDisplace ? "Yes" : "No");
			ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted("Displacement Force");
			ImGui::TableNextColumn(); ImGui::Text("%.2f", lp.m_displacementForce);

			ImGui::EndTable();
		}
	}

	// ---- 化学属性 ----
	bool hasChemical = def.m_isFlammable || def.m_isDissolve || def.m_isAcid ||
		def.m_isHighTemp || !def.m_isPersist;
	if (hasChemical && ImGui::CollapsingHeader("Chemical"))
	{
		if (ImGui::BeginTable("##ch", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
		{
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 140.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			auto brow = [](const char* label, bool val)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn(); ImGui::TextUnformatted(label);
					ImGui::TableNextColumn();
					if (val) ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Yes");
					else     ImGui::TextDisabled("No");
				};

			brow("Is High Temp", def.m_isHighTemp);
			brow("Is Persistent", def.m_isPersist);
			brow("Is Flammable", def.m_isFlammable);
			brow("Is Dissolve", def.m_isDissolve);
			brow("Is Acid", def.m_isAcid);
			brow("Is Corroded", def.m_isCorroded);

			ImGui::EndTable();
		}
	}

	// ---- 交互 ----
	if (ImGui::CollapsingHeader("Interaction"))
	{
		if (ImGui::BeginTable("##it", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
		{
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 140.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			const auto& ip = def.m_interaction;
			ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted("Penetration Resist.");
			ImGui::TableNextColumn(); ImGui::Text("%.1f", ip.m_penetrationResistance);
			ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted("Dissolution Rate");
			ImGui::TableNextColumn(); ImGui::Text("%.3f", ip.m_dissolutionRate);
			ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted("Is Permeable");
			ImGui::TableNextColumn(); ImGui::TextUnformatted(ip.m_isPermeable ? "Yes" : "No");
			ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted("Is Soluble");
			ImGui::TableNextColumn(); ImGui::TextUnformatted(ip.m_isSoluble ? "Yes" : "No");

			ImGui::EndTable();
		}
	}
}

// ============================================================
//  静态工具函数
// ============================================================
ImVec4 SandBoxUI::GetMaterialColor(CellMatType matType)
{
	auto it = CellMatManager::s_materialUIInfo.find(matType);
	if (it != CellMatManager::s_materialUIInfo.end())
		return it->second.m_color;
	return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
}

const char* SandBoxUI::GetPhysicsTypeName(PhyType physType)
{
	switch (physType) {
	case PhyType::PHY_STATIC_SOLID:       return "Static Solid";
	case PhyType::PHY_MOVE_SOLID:         return "Move Solid";
	case PhyType::PHY_LIQUID:             return "Liquid";
	case PhyType::PHY_CELLULAR_AUTOMATON: return "Cellular Automaton";
	default:                              return "Unknown";
	}
}

const char* SandBoxUI::GetMaterialDescription(CellMatType matType)
{
	auto it = CellMatManager::s_materialUIInfo.find(matType);
	if (it != CellMatManager::s_materialUIInfo.end())
		return it->second.m_description.c_str();
	return "Unknown Material";
}