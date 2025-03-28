#include "Tile.hpp"

Tile::Tile(int tileDefIndex, IntVec2 const& tileCoords) :m_tileDefIndex(tileDefIndex), m_tileCoordinates(tileCoords)
{
	m_bounds = GetTileBound();
}

AABB3 Tile::GetTileBound()
{
	AABB3 curBound = AABB3(Vec3((float)m_tileCoordinates.x, (float)m_tileCoordinates.y, 0.f),
		Vec3(m_tileCoordinates.x+1.f, m_tileCoordinates.y+1.f, 1.f));
	return curBound;
}
