#pragma once
#include <vector>
#include "Engine/Math/RaycastUtils.hpp"
#include "Game/Tile.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapDefinition.hpp"

class Game;
class Map;

enum class LayerType
{
	NONE,
	FLOOR,
	WALL,
	CEILING,
	DECORATION,
	OBJECT,
};

class MapLayer
{
public:
	MapLayer(Map* map, MapDefinition const& mapDef, LayerType layerType, std::string const& imagePath, std::vector<TileDefinition> const& tileDefSet,SpriteSheet* spriteSheet);
	virtual ~MapLayer();

	virtual void InitializeFromImage();
	virtual void GenerateEachTile(int columnIndex, int rowIndex, std::string& thisTileType);
	virtual void AddVerts();
	virtual void Update(float deltaSeconds);
	virtual void Render() const;

	std::string const& GetTileTypeNameByColor(Rgba8 const& pixelColor);
	int GetTileDefIndexByName(std::string const& typeName);

	Tile* GetTileByCoords(int coor_x, int coor_y);
	int GetTileIndexByCoords(int coor_x, int coor_y);
	bool IsCoordsLegal(int coordsX, int coordsY) const;

protected:
	LayerType m_layerType = LayerType::NONE;
	Map*	m_map = nullptr;
	SpriteSheet* m_spriteSheet = nullptr;
	std::vector<TileDefinition> const& m_defSets;
	MapDefinition const& m_mapDef;
	IntVec2 m_dimensions;
	std::vector<Tile> m_tiles;
	std::vector<Vertex_PCU> m_verts;
	std::string m_imagePath;
	float m_tileSizeX=1.f;
	float m_tileSizeY=1.f;
};