#include "World.hpp"
#include "Chunk.hpp"
#include "Player.hpp"

World::World(Player* player):m_player(player)
{
	InitializeChunks();
}

World::~World()
{
	for (Chunk* chunk : m_activeChunks)
	{
		delete chunk;
		chunk = nullptr;
	}
}

void World::Update(float deltaTime)
{
	m_player->Update(deltaTime);
	for (Chunk* chunk : m_activeChunks)
	{
		chunk->Update();
	}
}

void World::Render() const
{
	for (Chunk* chunk : m_activeChunks)
	{
		chunk->Render();
	}
}

void World::InitializeChunks()
{
	Chunk* chunk1 = new Chunk(IntVec2(0, 0));
	Chunk* chunk2 = new Chunk(IntVec2(2, 0));
	Chunk* chunk3 = new Chunk(IntVec2(2, 1));
	Chunk* chunk4 = new Chunk(IntVec2(2, -1));
	m_activeChunks.push_back(chunk1);
	m_activeChunks.push_back(chunk2);
	m_activeChunks.push_back(chunk3);
	m_activeChunks.push_back(chunk4);
}
