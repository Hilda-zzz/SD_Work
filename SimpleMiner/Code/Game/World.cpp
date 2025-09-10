#include "World.hpp"
#include "Chunk.hpp"
#include "Player.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Game/Game.hpp"

extern Game* g_theGame;

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

	Vec3 basisPos = m_player->m_position + m_player->m_orientation.GetForward_IFwd() * 50.f;
	Mat44 basisTransform = Mat44::MakeTranslation3D(basisPos);
	DebugAddWorldBasis(basisTransform, 0.f, DebugRenderMode::ALWAYS, 1.5f);

	DebugAddWorldBasis(Mat44(), 0.f, DebugRenderMode::USE_DEPTH, 1.f);
}

void World::Render() const
{
	for (Chunk* chunk : m_activeChunks)
	{
		chunk->Render();
	}
	if (m_isDebugDraw)
	{
		int totalChunkCount =(int) m_activeChunks.size();
		int totalVertsCount = 0;
		int totalIndicesCount = 0;
		for (Chunk* chunk : m_activeChunks)
		{
			chunk->RenderDebug(); 
			totalIndicesCount += chunk->GetIndicesCount();
			totalVertsCount += chunk->GetVertsCount();
		}
		char countBuffer[256];
		snprintf(countBuffer, sizeof(countBuffer),
			"Chunk: %d  Vertices: %d Indices: %d",
			totalChunkCount, totalVertsCount, totalIndicesCount);

		float textWidth = 700.f;
		float textHeight = 50.f;
		float margin = 10.f;
		Vec2 topRight = Vec2(margin + textWidth, g_theGame->GetScreenSize().y - margin);
		Vec2 bottomLeft = Vec2(margin, g_theGame->GetScreenSize().y - margin - textHeight);

		DebugAddScreenText(std::string(countBuffer),
			AABB2(bottomLeft,topRight),
			20.f,
			Vec2(1.f, 1.f),
			0.f,
			Rgba8::WHITE,
			Rgba8::WHITE);
	}
}

void World::ToggleDebugDraw()
{
	m_isDebugDraw = !m_isDebugDraw;
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
