#pragma once
#include <vector>
#include <string>
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/XmlUtils.hpp"

class TileDefinition
{
public:
	TileDefinition() = default;
	TileDefinition(XmlElement const* tileDefElement);
	static void InitializeTileDefinitionFromFile();
	static void InitializeWallDefinitionFromFile();
	~TileDefinition() {};

public:
	static std::vector<TileDefinition> s_tileDefinitions;
	static std::vector<TileDefinition> s_wallDefinitions;
	std::string m_name;
	bool m_isSolid;
	Rgba8 m_mapImagePixelColor;
	IntVec2 m_spriteCoords = IntVec2(-1, -1);
	//AABB2   m_UVs = AABB2::ZERO_TO_ONE;
};
