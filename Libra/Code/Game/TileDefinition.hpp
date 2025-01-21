#pragma once
#include <vector>
#include <string>
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/XmlUtils.hpp"

struct TileDefinition
{
	static std::vector<TileDefinition> s_tileDefinitions;
	Rgba8               m_mapImageColor = Rgba8(0, 0, 0, 0);
	int					m_spriteSheetIndex = 0;
	Rgba8				m_tintColor=Rgba8::WHITE;
	AABB2				m_UVs = AABB2::ZERO_TO_ONE;
	bool				m_isSolid = false;
	bool				m_isWater = false;
	bool				m_isDestruct = false;
	float				m_maxHealth = 999.f;
	std::string			m_name="NONDEF";

	TileDefinition() = default;
	TileDefinition(XmlElement const* tileDefElement);
	static void InitializeTileDefinitionFromFile();

	//#Todo: get tile def change to here , a static one
};
