#include "Game/Tile.hpp"

Tile::Tile(int coor_x, int coor_y, int typeIndex):m_tileCoordinates(IntVec2(coor_x,coor_y)),m_tileDefIndex(typeIndex)
{
	m_health = TileDefinition::s_tileDefinitions[typeIndex].m_maxHealth;
}
