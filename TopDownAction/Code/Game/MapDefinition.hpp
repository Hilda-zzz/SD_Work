#pragma once
#include <string>
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "Engine/Core/XmlUtils.hpp"
struct MapDefinition
{
	static std::vector<MapDefinition> s_mapDefinitions;
	static void InitializeMapDefinitionFromFile();
	MapDefinition() = default;
	MapDefinition(XmlElement const* mapDefElement);
	//----------------------------------------------------
	std::string m_name = "NONDEF";
	std::string m_mapImageName = "";
	std::string m_mapWallImageName = "";
	IntVec2 m_dimensions = IntVec2(-1, -1);
// 	IntVec2 m_startCoords = IntVec2(-1, -1);
// 	IntVec2 m_exitCoords = IntVec2(-1, -1);
};