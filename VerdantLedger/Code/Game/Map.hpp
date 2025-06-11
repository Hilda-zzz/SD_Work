#pragma once

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
	void TileMapRender() const;

private:
	Game* m_game = nullptr;
	TileMap* m_tileMap = nullptr;
	Player* m_player = nullptr;
};