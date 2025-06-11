#include "Map.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Player.hpp"
#include "GameCommon.hpp"
#include "Game/TileMap.hpp"

extern Renderer* g_theRenderer;

Map::Map(Game* game, TileMap* tileMap, Player* player):
	m_game(game),m_tileMap(tileMap),m_player(player)
{
}

Map::~Map()
{
}

void Map::Update(float deltaSeconds)
{
	m_player->Update(deltaSeconds);
}

void Map::Render() const
{
	g_theRenderer->BeginCamera(m_player->m_gameplayCam);

	m_tileMap->Render();

	DebugDrawRing(0.5f, 10.f, Rgba8::WHITE, Vec2(0.f, 0.f));
	m_player->Render();
	
	g_theRenderer->EndCamera(m_player->m_gameplayCam);
}

void Map::TileMapRender() const
{
}
