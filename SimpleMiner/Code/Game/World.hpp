#pragma once
#include <vector>
class Chunk;
class Player;

class World
{
public:
	World(Player* player);
	~World();

	void Update(float deltaTime);
	void Render() const;

	void ToggleDebugDraw();
private:
	void InitializeChunks();
public:
private:
	std::vector<Chunk*> m_activeChunks;
	Player* m_player = nullptr;
	bool m_isDebugDraw = false;
};