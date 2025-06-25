#include "GroundObstacle.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/VertexUtils.hpp"

extern Renderer* g_theRenderer;

GroundObstacle::GroundObstacle(ObstacleType type, IntVec2 gridPos):m_type(type),m_gridPos(gridPos)
{
	AABB2 uv;
	switch (type) {
	case ObstacleType::TREE: 
		m_health = 100; 
		break;
	case ObstacleType::ROCK: 
		m_health = 50; 
		m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/FarmTinyAssetPack/Props/Spring/spring stones.png");
		m_spriteSheet = new SpriteSheet(*m_texture, IntVec2(16, 10));
		m_spriteIndex = 17;
		uv = m_spriteSheet->GetSpriteUVs(m_spriteIndex);
		AddVertsForAABB2D(m_verts, AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)), Rgba8::WHITE, uv.m_mins, uv.m_maxs);
		break;
	case ObstacleType::WEED: 
		m_health = 10; 
		break;
	case ObstacleType::LOG: 
		m_health = 30; 
		break;
	default: 
		m_health = 10; 
		break;
	}
}

GroundObstacle::~GroundObstacle()
{
	delete m_spriteSheet;
	m_spriteSheet = nullptr;
}

void GroundObstacle::Render() const
{
	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->SetModelConstants(Mat44::MakeTranslation2D(
		Vec2((float)m_gridPos.x, (float)m_gridPos.y)
	));
	g_theRenderer->DrawVertexArray(m_verts);
}
