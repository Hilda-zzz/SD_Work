//#pragma once
//#pragma once
//#include "Engine/Core/EngineCommon.hpp"
//#include "CellMatDef.hpp"
//#include "CellMatManager.hpp"
//#include <ThirdParty/imgui/imgui.h>
//#include "CellMatBrush.hpp"
//
//class SandboxPlayer;
//
//class SandBoxUI 
//{
//public:
//	SandBoxUI();
//	~SandBoxUI();
//
//	// main panel
//	void RenderMaterialBrushUI(SandboxPlayer* player);
//
//	// sub panel
//	void RenderMaterialSelector(SandboxPlayer* player);
//	void RenderMaterialProperties(CellMatType selectedMaterial);
//
//	// help func
//	static ImVec4 GetMaterialColor(CellMatType matType);
//	static const char* GetPhysicsTypeName(PhyType physType);
//	static const char* GetMaterialDescription(CellMatType matType);
//
//	// RB drawing and generation
//	void RenderRigidBodyPanel(SandboxPlayer* player);
//	bool IsInRigidBodyDrawMode() const { return m_rigidBodyDrawMode; }
//	CellMatType GetRigidBodyMaterial() const { return m_rigidBodyMaterial; }
//
//private:
//	// help
//	void InitializeMaterialUIInfo();
//	void RenderMaterialButton(CellMatType matType, bool isSelected, SandboxPlayer* player);
//	void RenderPhysicsGroup(const char* groupName, PhyType physType,
//		const std::vector<CellMatType>& materials,
//		SandboxPlayer* player);
//	void RenderParameterTable(const CellMatDef& matDef);
//
//private:
//	// UI state
//	bool m_showMaterialSelector = true;
//	bool m_showBrushControls = true;
//	bool m_showMaterialProperties = true;
//
//	//static std::unordered_map<CellMatType, CellMatUIInfo> s_materialUIInfo;
//
//	// Material selector state
//	PhyType m_selectedCategory = PhyType::PHY_MOVE_SOLID; // Default to Move Solid
//	bool m_showToolsOnly = false;
//
//	// Rigid Body Drawing Mode
//	bool m_rigidBodyDrawMode = false;
//	bool m_showRigidBodyPanel = true;
//	CellMatType m_rigidBodyMaterial = CellMatType::MAT_WOOD; 
//
//};

#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "CellMatDef.hpp"
#include "CellMatManager.hpp"
#include <ThirdParty/imgui/imgui.h>
#include "CellMatBrush.hpp"

class SandboxPlayer;
class SandboxMap;

// ============================================================
//  SandBoxUI  —  统一侧边栏版本
//
//  布局：
//   左侧固定侧边栏（~270px）包含
//     Tab 1  Brush   — 笔刷 & 材质选择 & 材质属性
//     Tab 2  Debug   — 可视化 / Chunk 调试
//     Tab 3  Stats   — 统计信息
//     Tab 4  RBody   — 刚体创建
//
//   右侧可选浮窗
//     Cell Inspector — 鼠标悬停格子实时信息（可切换显示）
// ============================================================
class SandBoxUI
{
public:
	SandBoxUI();
	~SandBoxUI();

	// ---- 主入口 ----
	// 每帧从 SandboxMap::Render() 调用一次
	void RenderAll(SandboxPlayer* player, SandboxMap* map);

	// ---- Rigid Body 状态查询（供 SandboxPlayer 使用）----
	bool IsInRigidBodyDrawMode() const { return m_rigidBodyDrawMode; }
	CellMatType GetRigidBodyMaterial() const { return m_rigidBodyMaterial; }

	// ---- 旧接口保留（避免 SandboxMap 大改）----
	void RenderMaterialBrushUI(SandboxPlayer* player);   // 内部转发给 RenderAll
	void RenderRigidBodyPanel(SandboxPlayer* player);    // 内部转发给 RenderAll

	static ImVec4 GetMaterialColor(CellMatType matType);
	static const char* GetPhysicsTypeName(PhyType physType);
	static const char* GetMaterialDescription(CellMatType matType);

private:
	// ---- Sidebar Tabs ----
	void RenderTab_Brush(SandboxPlayer* player);
	void RenderTab_Debug(SandboxMap* map);
	void RenderTab_Stats(SandboxMap* map);
	void RenderTab_RigidBody(SandboxPlayer* player, SandboxMap* map);

	// ---- Brush Tab 子区域 ----
	void RenderBrushControls(SandboxPlayer* player);
	void RenderCategoryBar(SandboxPlayer* player);
	void RenderMaterialGrid(SandboxPlayer* player);
	void RenderMaterialProperties(CellMatType selectedMaterial);

	// ---- Cell Inspector 浮窗 ----
	void RenderCellInspector(SandboxMap* map);

	// ---- Debug Tab 子区域 ----
	void RenderDebugGrid(SandboxMap* map);
	void RenderDebugColorMode(SandboxMap* map);
	void RenderDebugRigidBody(SandboxMap* map);

	// ---- 工具函数 ----
	void RenderParameterTable(const CellMatDef& matDef);
	void RenderMaterialButton(CellMatType matType, bool isSelected, SandboxPlayer* player);

	static ImVec4 BrightenColor(ImVec4 c, float factor);
	static ImVec4 DarkenColor(ImVec4 c, float factor);

private:
	// ---- 侧边栏状态 ----
	int   m_activeTab = 0;   // 0=Brush 1=Debug 2=Stats 3=RBody
	float m_sidebarWidth = 270.0f;
	bool  m_sidebarCollapsed = false;

	// ---- Brush Tab 状态 ----
	PhyType m_selectedCategory = PhyType::PHY_MOVE_SOLID;
	bool    m_showToolsOnly = false;
	bool    m_propertiesOpen = true;   // CollapsingHeader open/close

	// ---- Cell Inspector ----
	bool m_showCellInspector = true;

	// ---- Rigid Body 状态 ----
	bool        m_rigidBodyDrawMode = false;
	CellMatType m_rigidBodyMaterial = CellMatType::MAT_WOOD;
};