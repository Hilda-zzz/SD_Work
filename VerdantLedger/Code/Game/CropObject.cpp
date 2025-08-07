#include "CropObject.hpp"
#include "CropDefinitions.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern Renderer* g_theRenderer;

CropObject::CropObject(CropDefinitions* curCropDef, IntVec2 gridPos):m_cropDef(curCropDef),m_gridPos(gridPos)
{
	if (curCropDef)
	{
		m_spriteSheet = m_cropDef->m_spriteSheet;
		m_spriteGridPos = curCropDef->m_spriteStartGridPos;
		m_spriteIndex=m_spriteSheet->GetSpriteIndexFromGridPos(m_spriteGridPos);

		// Add verts
		m_cropVerts.clear();
		m_cropVerts.reserve(6);
		AABB2 uv = m_spriteSheet->GetSpriteUVs(m_spriteIndex);
		AABB2 bound = AABB2(Vec2((float)m_gridPos.x, (float)m_gridPos.y + 0.2f),
			Vec2((float)m_gridPos.x, (float)m_gridPos.y+0.2f) + Vec2(1.f, 1.f));
		AddVertsForAABB2D(m_cropVerts, bound, Rgba8::WHITE, uv.m_mins, uv.m_maxs, (float)m_gridPos.y + 0.2f + Z_OFFSET);

		m_harvestIconVerts.clear();
		m_harvestIconVerts.reserve(6);
		AABB2 uv_harvest = m_spriteSheet->GetSpriteUVs(m_spriteIndex+9);
		AABB2 bound_harvest = AABB2(bound.m_mins+Vec2(0.f,0.9f), bound.m_maxs + Vec2(0.f, 0.9f));
		AddVertsForAABB2D(m_harvestIconVerts, bound_harvest, Rgba8::WHITE, uv_harvest.m_mins, uv_harvest.m_maxs, (float)m_gridPos.y + 0.2f + Z_OFFSET);
	}
}

CropObject::~CropObject()
{
}

void CropObject::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void CropObject::Render() const
{
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(&m_spriteSheet->GetTexture());
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_theRenderer->DrawVertexArray(m_cropVerts);

	if (m_canHarvest)
	{
		g_theRenderer->DrawVertexArray(m_harvestIconVerts);
	}
}

void CropObject::SettleDailyState()
{
	if (m_hasWater)
	{
		m_curState++;
		if (m_curState >= m_cropDef->m_matureDay)
		{
			m_canHarvest = true;
		}

		// update verts
		if (!m_canHarvest)
		{
			m_spriteGridPos.x++;
			m_spriteIndex = m_spriteSheet->GetSpriteIndexFromGridPos(m_spriteGridPos);
			m_cropVerts.clear();
			AABB2 uv = m_spriteSheet->GetSpriteUVs(m_spriteIndex);
			AABB2 bound = AABB2(Vec2((float)m_gridPos.x, (float)m_gridPos.y + 0.2f),
				Vec2((float)m_gridPos.x, (float)m_gridPos.y + 0.2f) + Vec2(1.f, 1.f));
			AddVertsForAABB2D(m_cropVerts, bound, Rgba8::WHITE, uv.m_mins, uv.m_maxs, (float)m_gridPos.y + 0.2f + Z_OFFSET);
		}
	}
	m_hasWater = false;
}
