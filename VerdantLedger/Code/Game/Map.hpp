#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"

class Game;
class TileMap;
class Player;

class Map
{
public:
	Map(Game* game, TileMap* tileMap,Player* player);
	~Map();

	void Update(float deltaSeconds);

	void Render() const;

private:
	void CheckPlayerCollWithSolidTiles();

	IntVec2 GetTileCoordsFromPoint(Vec2 const& point);

	void PushOutOfEachTile(IntVec2 tileCoords, Vec2& entityPos, float entityPhyRadius);

	void TileMapRender() const;

private:
	Game* m_game = nullptr;
	TileMap* m_tileMap = nullptr;
	Player* m_player = nullptr;
};