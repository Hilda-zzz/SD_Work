#pragma once
#include <string>
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "Engine/Core/XmlUtils.hpp"
struct MapDefinition
{
	static std::vector<MapDefinition> s_mapDefinitions;

	std::string m_name = "NONDEF";
	std::string m_mapImageName = "";
	IntVec2 m_dimensions = IntVec2(-1, -1);
	IntVec2 m_startCoords = IntVec2(-1, -1);
	IntVec2 m_exitCoords = IntVec2(-1, -1);
	std::string m_fillTileType = "NONDEF";
	std::string	m_edgeTileType = "NONDEF";
	std::string	m_worm1TileType = "NONDEF"; int m_worm1Count = -1; int  m_worm1MaxLen = -1;
	std::string	m_worm2TileType = "NONDEF"; int  m_worm2Count = -1; int  m_worm2MaxLen = -1;
	std::string	m_waterTileType = "NONDEF"; int  m_wormWaterCount = -1; int  m_wormWaterMaxLen = -1;
	std::string	m_startFloorTileType = "NONDEF"; 
	std::string	m_startBunkerTileType = "NONDEF";
	std::string	m_endFloorTileType = "NONDEF";
	std::string	m_endBunkerTileType = "NONDEF";
	std::string	m_startTileType = "NONDEF";
	std::string	m_endTileType = "NONDEF";
	int m_rubbleCount = -1;
	int m_leoCount = -1;
	int m_ariesCount = -1;
	int m_scorpioCount = -1;
	int m_capricornCount = -1;
	int m_treeCount = -1;
	MapDefinition() = default;
	MapDefinition(XmlElement const* mapDefElement);
	static void InitializeMapDefinitionFromFile();
};