#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/Timer.hpp"

class Game;
class TileMap;
class Player;
class TileChunk;


class Map
{
public:
	Map(Game* game, TileMap* tileMap,Player* player);
	~Map();

	void Update(float deltaSeconds);

	void Render() const;

	IntVec2 GetTileCoordsFromPoint(Vec2 const& point);

	void UsingToolTowardsGridPos(IntVec2 const& aimGridPos, PlayerTools toolType);

private:
	void UpdateCamFollow(float deltaSecondes);

	void UpdateDelayDirtyObstacleChunk();
	void UpdateDelayDirtyFarmlandChunk();

	void CheckPlayerCollWithSolidTiles();

	void PushOutOfEachTile(IntVec2 tileCoords, Vec2& entityPos, float entityPhyRadius);

	void TileMapRender() const;

public:
	Camera  m_gameplayCam;
private:
	Game* m_game = nullptr;
	TileMap* m_tileMap = nullptr;
	Player* m_player = nullptr;
	Vec2 m_playerPrevPos = Vec2::ZERO;
	TileChunk* m_curDirtyChunk = nullptr;
	Timer m_dirtyObstacleDelayTimer;
	Timer m_dirtyFarmlandDelayTimer;
};