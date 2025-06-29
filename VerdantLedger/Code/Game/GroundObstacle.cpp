#include "GroundObstacle.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/ObstacleDefinitions.hpp"
#include "Game.hpp"

extern Renderer* g_theRenderer;

GroundObstacle::GroundObstacle(ObstacleType type, ObstacleDefinition* curDef,IntVec2 gridPos, int spriteIndex)
	:m_type(type),m_obstacleDef(curDef),m_gridPos(gridPos),m_spriteSheet(curDef->m_spriteSheet),m_spriteIndex(spriteIndex)
{
// 	AABB2 uv;
// 	switch (type) {
// 	case ObstacleType::TREE: 
// 		m_durability = 100; 
// 		break;
// 	case ObstacleType::ROCK: 
// 		m_durability = 50; 
// 		m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/FarmTinyAssetPack/Props/Spring/spring stones.png");
// 		m_spriteSheet = new SpriteSheet(*m_texture, IntVec2(16, 10));
// 		m_spriteIndex = 17;
// 		uv = m_spriteSheet->GetSpriteUVs(m_spriteIndex);
// 		AddVertsForAABB2D(m_verts, AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)), Rgba8::WHITE, uv.m_mins, uv.m_maxs);
// 		break;
// 	case ObstacleType::WEED: 
// 		m_durability = 10; 
// 		break;
// 	case ObstacleType::LOG: 
// 		m_durability = 30; 
// 		break;
// 	default: 
// 		m_durability = 10; 
// 		break;
// 	}
	m_transparentTimer = Timer(0.1f);;

	AABB2 spriteUV = m_spriteSheet->GetSpriteUVs(spriteIndex);
	Vec2 spriteSize = m_obstacleDef->m_spriteSheet->GetEachSpriteWidthHeight();
	Vec2 spriteSizeInWorld = spriteSize / (float)RESOLUTION;

	Vec2 gridBottomCenter = Vec2((float)gridPos.x + 0.5f, (float)gridPos.y);
	Vec2 spriteBottomLeft = gridBottomCenter - Vec2(spriteSizeInWorld.x * 0.5f, 0.f);
	Vec2 spriteTopRight = gridBottomCenter + Vec2(spriteSizeInWorld.x * 0.5f, spriteSizeInWorld.y);

	AddVertsForAABB2D(m_verts,
		AABB2(spriteBottomLeft, spriteTopRight),
		Rgba8(255, 255, 255, 255), spriteUV.m_mins, spriteUV.m_maxs, (float)gridPos.y + Z_OFFSET);
}

GroundObstacle::~GroundObstacle()
{
	delete m_spriteSheet;
	m_spriteSheet = nullptr;
}

void GroundObstacle::Update()
{
	Rgba8 newColor;
	if (!m_transparentTimer.IsStopped())
	{
		if (m_isTurningToTransparent)
		{
			newColor = Interpolate(Rgba8::WHITE, Rgba8(255, 255, 255, 100), m_transparentTimer.GetElapsedFraction());
		}
		else if (m_isTurningToOpaque)
		{
			newColor = Interpolate(Rgba8(255, 255, 255, 100), Rgba8::WHITE, m_transparentTimer.GetElapsedFraction());
		}

		if (m_transparentTimer.GetElapsedFraction() >= 1)
		{
			m_transparentTimer.Stop();
			if (m_isTurningToTransparent)
			{
				newColor = Rgba8(255, 255, 255, 100);
				m_isTurningToTransparent = false;
				m_isTransparent = true;
			}
			else if (m_isTurningToOpaque)
			{
				newColor = Rgba8::WHITE;
				m_isTurningToOpaque = false;
				m_isTransparent = false;
			}
		}
	}
	else
	{
		if (m_isTransparent)
		{
			newColor = Rgba8(255, 255, 255, 100);
		}
		else
		{
			newColor = Rgba8::WHITE;
		}
	}

	for (Vertex_PCU& vert : m_verts)
	{
		vert.m_color = newColor;
	}
}

void GroundObstacle::Render() const
{
	g_theRenderer->BindTexture(&m_spriteSheet->GetTexture());
// 	g_theRenderer->SetModelConstants(Mat44::MakeTranslation2D(
// 		Vec2((float)m_gridPos.x, (float)m_gridPos.y)
// 	));
	if (m_isTransparent)
	{
		g_theRenderer->SetDepthMode(DepthMode::READ_ONLY_LESS_EQUAL);
	}
	g_theRenderer->DrawVertexArray(m_verts);
	if (m_isTransparent)
	{
		g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	}
}

void GroundObstacle::SetTransparent(bool aimTransparent)
{
	// 如果动画时间过长的话，玩家在一段播放之前就已经离开，会出现问题
	if (m_isTransparent == aimTransparent) return; 

	if (aimTransparent)
	{
		m_isTurningToTransparent= true;
		m_isTurningToOpaque = false;
	}
	else
	{
		m_isTurningToTransparent = false;
		m_isTurningToOpaque = true;
	}

	m_transparentTimer.Start();
}
