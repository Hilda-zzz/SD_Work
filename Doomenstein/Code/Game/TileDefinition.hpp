#pragma once
#include <string>
#include "GameCommon.hpp"
#include "Engine/Math/AABB2.hpp"

class TileDefinition
{
public:
	TileDefinition()=default;
	TileDefinition(XmlElement const* tileDefElement);
	static void InitializeTileDefinitionFromFile();
	~TileDefinition() {};

public:
	static std::vector<TileDefinition> s_tileDefinitions;
	std::string m_name;
	bool m_isSolid;
	Rgba8 m_mapImagePixelColor;
	IntVec2 m_floorSpriteCoords=IntVec2(-1,-1);
	IntVec2 m_ceilingSpriteCoords = IntVec2(-1, -1);
	IntVec2 m_wallSpriteCoords = IntVec2(-1, -1);
	//AABB2   m_UVs = AABB2::ZERO_TO_ONE;
};