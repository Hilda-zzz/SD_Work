#pragma once
#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "CellMatDef.hpp"
#include "CellMatManager.hpp"
#include <ThirdParty/imgui/imgui.h>
#include "CellMatBrush.hpp"

class SandboxPlayer;

class SandBoxUI 
{
public:
	SandBoxUI();
	~SandBoxUI();

	// main panel
	void RenderMaterialBrushUI(SandboxPlayer* player);

	// sub panel
	void RenderMaterialSelector(SandboxPlayer* player);
	void RenderMaterialProperties(CellMatType selectedMaterial);

	// help func
	static ImVec4 GetMaterialColor(CellMatType matType);
	static const char* GetPhysicsTypeName(PhyType physType);
	static const char* GetMaterialDescription(CellMatType matType);

	// RB drawing and generation
	void RenderRigidBodyPanel(SandboxPlayer* player);
	bool IsInRigidBodyDrawMode() const { return m_rigidBodyDrawMode; }
	CellMatType GetRigidBodyMaterial() const { return m_rigidBodyMaterial; }

private:
	// help
	void InitializeMaterialUIInfo();
	void RenderMaterialButton(CellMatType matType, bool isSelected, SandboxPlayer* player);
	void RenderPhysicsGroup(const char* groupName, PhyType physType,
		const std::vector<CellMatType>& materials,
		SandboxPlayer* player);
	void RenderParameterTable(const CellMatDef& matDef);

private:
	// UI state
	bool m_showMaterialSelector = true;
	bool m_showBrushControls = true;
	bool m_showMaterialProperties = true;

	//static std::unordered_map<CellMatType, CellMatUIInfo> s_materialUIInfo;

	// Material selector state
	PhyType m_selectedCategory = PhyType::PHY_MOVE_SOLID; // Default to Move Solid
	bool m_showToolsOnly = false;

	// Rigid Body Drawing Mode
	bool m_rigidBodyDrawMode = false;
	bool m_showRigidBodyPanel = true;
	CellMatType m_rigidBodyMaterial = CellMatType::MAT_WOOD; 

};