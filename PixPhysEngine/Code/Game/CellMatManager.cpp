#include "Game/CellMatManager.hpp"
#include "ThirdParty/tracy/tracy/Tracy.hpp"

std::unordered_map<CellMatType, CellMatDef> CellMatManager::s_materialDefs;

std::unordered_map<CellMatType, CellMatUIInfo> CellMatManager::s_materialUIInfo;

CellMatManager::CellMatManager()
{

}

void CellMatManager::InitializeMaterialUIInfo()
{
	for (const auto& pair : CellMatManager::GetAllMaterialDefs()) 
	{
		CellMatType matType = pair.first;
		const CellMatDef& matDef = pair.second;

		s_materialUIInfo[matType].m_name = matDef.m_name;
		s_materialUIInfo[matType].m_description = matDef.m_description;
		s_materialUIInfo[matType].m_physType = matDef.m_physicsType;
		s_materialUIInfo[matType].m_physTypeName = GetPhysicsTypeName(matDef.m_physicsType);

		// 颜色转换
		Rgba8 m_color = matDef.m_colorMin;
		s_materialUIInfo[matType].m_color = ImVec4(
			m_color.r / 255.0f, m_color.g / 255.0f, m_color.b / 255.0f, 1.0f
		);
	}
}

const std::unordered_map<CellMatType, CellMatDef>& CellMatManager::GetAllMaterialDefs()
{
	//ZoneScoped;
	return s_materialDefs;
}

const char* CellMatManager::GetPhysicsTypeName(PhyType physType)
{
	switch (physType) {
	case PhyType::PHY_STATIC_SOLID: return "Static Solid";
	case PhyType::PHY_MOVE_SOLID: return "Move Solid";
	case PhyType::PHY_LIQUID: return "Liquid";
	default: return "Unknown";
	}
}
