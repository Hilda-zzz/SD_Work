#pragma once
#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "CellMatDef.hpp"
#include "CellMatManager.hpp"
#include <ThirdParty/imgui/imgui.h>

class SandboxPlayer;

struct CellMatUIInfo {
	PhyType m_physType;
	std::string m_physTypeName;

	std::string m_name;
	std::string m_description;
	ImVec4 m_color;
};

class SandBoxUI 
{
public:
	SandBoxUI();
	~SandBoxUI();

	// main panel
	void RenderMaterialBrushUI(SandboxPlayer* player);

	// sub panel
	void RenderMaterialSelector(SandboxPlayer* player);
	void RenderBrushControls(SandboxPlayer* player);
	void RenderMaterialProperties(CellMatType selectedMaterial);

	// help func
	static ImVec4 GetMaterialColor(CellMatType matType);
	static const char* GetPhysicsTypeName(PhyType physType);
	static const char* GetMaterialDescription(CellMatType matType);

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

	static std::unordered_map<CellMatType, CellMatUIInfo> s_materialUIInfo;

};