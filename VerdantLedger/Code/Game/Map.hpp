#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/Timer.hpp"
#include "ChunkUpdateManager.hpp"

class Game;
class TileMap;
class Player;
class TileChunk;
class InventorySlotButton;

class Map
{
public:
	Map(Game* game, TileMap* tileMap,Player* player);
	~Map();

	void Update(float deltaSeconds);

	void Render() const;

	IntVec2 GetTileCoordsFromPoint(Vec2 const& point);

	void UsingToolTowardsGridPos(IntVec2 const& aimGridPos, PlayerTools toolType,InventorySlotButton& curInventoryBtn);
	bool HarvestTowardsGridPos(IntVec2 const& aimGridPos);
	TileMap* GetCurTileMap() { return m_tileMap; }

private:
	void UpdateCamFollow();

	void UpdateVisibleChunk(float deltaSeconds);

	void CheckPlayerCollWithSolidTiles();

	void PushOutOfEachTile(IntVec2 tileCoords, Vec2& entityPos, float entityPhyRadius);

	void TileMapRender() const;

	std::string GetObstacleName(ObstacleType type);
public:
	Camera  m_gameplayCam;
	AABB2 m_transMapTriggerBox = AABB2();
	Vec2 m_playerStartPos;
	Map* m_transToMap = nullptr;
	bool m_isInside = false;
private:
	Game* m_game = nullptr;
	TileMap* m_tileMap = nullptr;
	Player* m_player = nullptr;
	ChunkUpddateManger m_chunkUpdateManager;
	Vec2 m_playerPrevPos = Vec2::ZERO;
	std::vector<IntVec2> m_visibleChunk;

	Vec2 m_camBound = Vec2(0.f, 0.f);

	
};