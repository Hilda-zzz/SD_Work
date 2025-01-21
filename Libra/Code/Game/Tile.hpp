#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Game/TileDefinition.hpp"

//CHANGE TO
//flyweight DESIGN PATTERN
//struct
//struct Tile
//{
//	IntVec2 m_tileCoordinates=IntVec2(-1,-1);
//	TileType m_tileType=TILE_TYPE_INVALID;
//};
class Tile
{
public:
	Tile(int coor_x,int coor_y,int typeIndex);
	~Tile() {};
public:
	IntVec2 m_tileCoordinates=IntVec2(-1,-1);
	int m_tileDefIndex = -1;
	float m_health;
};