#include "Game/Map.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Player.hpp"
#include "GameCommon.hpp"
#include "Game/TileMap.hpp"
#include "Game/TileMapManager.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Window/Window.hpp"
#include "Game/TileTypesInGame.hpp"

extern Renderer* g_theRenderer;
extern Window* g_theWindow;

Map::Map(Game* game, TileMap* tileMap, Player* player):
	m_game(game),m_tileMap(tileMap),m_player(player)
{
	IntVec2 windowDimension = g_theWindow->GetClientDimensions();
	m_gameplayCam.SetViewport(AABB2(Vec2(0.f, 0.f), Vec2((float)windowDimension.x, (float)windowDimension.y)));
	m_gameplayCam.SetOrthographicView(m_player->m_position - Vec2(30.f, 15.f), m_player->m_position + Vec2(30.f, 15.f));
}

Map::~Map()
{
}

void Map::Update(float deltaSeconds)
{
	m_player->Update(deltaSeconds);
	CheckPlayerCollWithSolidTiles();
	m_gameplayCam.SetOrthographicView(m_player->m_position - Vec2(30.f, 15.f), m_player->m_position + Vec2(30.f, 15.f));
}

void Map::Render() const
{
	g_theRenderer->BeginCamera(m_gameplayCam);

	m_tileMap->Render();

	m_player->Render();

	g_theRenderer->EndCamera(m_gameplayCam);
}

void Map::CheckPlayerCollWithSolidTiles()
{
	if (m_player)
	{
 		IntVec2 entityTileCoordsPos = GetTileCoordsFromPoint(m_player->m_position);
 		//N
  		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x, entityTileCoordsPos.y + 1), m_player->m_position, m_player->m_physicsRadius);
  		//S                                                                                                
  		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x, entityTileCoordsPos.y - 1), m_player->m_position, m_player->m_physicsRadius);
  		//W                                                                                                
  		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x - 1, entityTileCoordsPos.y), m_player->m_position, m_player->m_physicsRadius);
  		//E                                                                                               
  		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x + 1, entityTileCoordsPos.y), m_player->m_position, m_player->m_physicsRadius);
  		//NE
		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x + 1, entityTileCoordsPos.y + 1), m_player->m_position, m_player->m_physicsRadius);
		//NW
		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x + 1, entityTileCoordsPos.y - 1), m_player->m_position, m_player->m_physicsRadius);
		//SE
		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x - 1, entityTileCoordsPos.y + 1), m_player->m_position, m_player->m_physicsRadius);
		//SW
		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x - 1, entityTileCoordsPos.y - 1), m_player->m_position, m_player->m_physicsRadius);
	}
}

IntVec2 Map::GetTileCoordsFromPoint(Vec2 const& point)
{
    return IntVec2(static_cast<int>(floorf(point.x)), static_cast<int>(floorf(point.y)));
}

void Map::PushOutOfEachTile(IntVec2 tileCoords, Vec2& entityPos, float entityPhyRadius)
{
    uint32_t gid= m_tileMap->GetTileGidFromLayerID(m_tileMap->m_markLayerId, tileCoords);
	uint32_t flag =m_game->g_tileManager->m_gidToTilePropertyFlag[gid];
	if (flag & static_cast<uint32_t>(TerrainType::SOLID))
	{
		AABB2 thisTileBox = AABB2(static_cast<float>(tileCoords.x), static_cast<float>(tileCoords.y),
			static_cast<float>(tileCoords.x + 1.f), static_cast<float>(tileCoords.y + 1));
		PushDiscOutOfAABB2D(entityPos, entityPhyRadius, thisTileBox);
	}
}

void Map::TileMapRender() const
{
}
