#include "MapLayer.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/Image.hpp"
#include <Engine/Core/VertexUtils.hpp>

MapLayer::MapLayer(Map* map, MapDefinition const& mapDef, LayerType layerType, 
	std::string const& imagePath, std::vector<TileDefinition> const& tileDefSet,SpriteSheet* spriteSheet)
	:m_map(map),m_mapDef(mapDef),m_layerType(layerType),m_defSets(tileDefSet),m_imagePath(imagePath),m_spriteSheet(spriteSheet)
{
	m_dimensions = map->m_dimensions;
}

MapLayer::~MapLayer()
{
}

void MapLayer::InitializeFromImage()
{
	m_tiles.reserve(m_dimensions.x * m_dimensions.y);
	Image* mapImage = g_theRenderer->CreateImageFromFile(m_imagePath.c_str());
	for (int rowIndex = 0; rowIndex < m_dimensions.y; rowIndex++)
	{
		for (int columnIndex = 0; columnIndex < m_dimensions.x; columnIndex++)
		{
			Rgba8 pixelColor = mapImage->GetTexelColor(IntVec2(columnIndex, rowIndex));
			std::string thisTileType = GetTileTypeNameByColor(pixelColor);
			if (thisTileType!= "NONE")
			{
				GenerateEachTile(columnIndex, rowIndex, thisTileType);
			}
		}
	}
	delete mapImage;
	mapImage = nullptr;
	AddVerts();
}

void MapLayer::GenerateEachTile(int columnIndex, int rowIndex, std::string& thisTileType)
{
	Tile newTile = Tile(columnIndex, rowIndex, GetTileDefIndexByName(thisTileType));
	m_tiles.push_back(newTile);
}

void MapLayer::AddVerts()
{
	for (int i = 0; i < m_tiles.size(); i++)
	{
		Vec2 worldBL_ofThisTile = Vec2(m_tiles[i].m_tileCoordinates.x * m_tileSizeX, (float)m_tiles[i].m_tileCoordinates.y * m_tileSizeY);
		AABB2 aabb_thisTile = AABB2(worldBL_ofThisTile, worldBL_ofThisTile + Vec2(m_tileSizeX, m_tileSizeY));
		if (m_tiles[i].m_tileDefIndex != -1)
		{
			TileDefinition curDef = TileDefinition::s_tileDefinitions[m_tiles[i].m_tileDefIndex];
			int spriteIndex = curDef.m_spriteCoords.x
				+ (curDef.m_spriteCoords.y * m_spriteSheet->m_gridLayout.x);
			AABB2 tileUV = m_spriteSheet->GetSpriteUVs(spriteIndex);
			AddVertsForAABB2D(m_verts,
				aabb_thisTile,
				Rgba8::WHITE,
				tileUV.m_mins,
				tileUV.m_maxs);
		}
	}
}

void MapLayer::Update(float deltaSeconds)
{
}

void MapLayer::Render() const
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetModelConstants(Mat44(), Rgba8::WHITE);
	g_theRenderer->BindTexture(&m_spriteSheet->GetTexture());
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->DrawVertexArray(m_verts);
}

std::string const& MapLayer::GetTileTypeNameByColor(Rgba8 const& pixelColor)
{
	// TODO: insert return statement here
	return "";
}

int MapLayer::GetTileDefIndexByName(std::string const& typeName)
{
	return 0;
}

Tile* MapLayer::GetTileByCoords(int coor_x, int coor_y)
{
	return nullptr;
}

int MapLayer::GetTileIndexByCoords(int coor_x, int coor_y)
{
	return 0;
}

bool MapLayer::IsCoordsLegal(int coordsX, int coordsY) const
{
	return false;
}
