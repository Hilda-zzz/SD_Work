#pragma once
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVec2.hpp"
class TileDefinition;
class Tile
{
public:
	Tile(int tileDefIndex, IntVec2 const& tileCoords);
	AABB3 GetTileBound();

	AABB3 m_bounds;
	IntVec2 m_tileCoordinates = IntVec2(-1, -1);
	int m_tileDefIndex = -1;
};