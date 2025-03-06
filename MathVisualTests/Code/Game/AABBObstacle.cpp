#include "AABBObstacle.hpp"
#include <Engine/Core/Rgba8.hpp>
#include <vector>
#include <Engine/Core/Vertex_PCU.hpp>
#include <Engine/Core/VertexUtils.hpp>
#include "Game/GameCommon.hpp"

Rgba8 NORMAL_AABB_COLOR = Rgba8(0, 100, 225, 255);
Rgba8 BRIGHT_AABB_COLOR = Rgba8(150, 100, 225, 255);

AABBObstacle::AABBObstacle(AABB2& box):m_AABB2(box),m_isRaycast(false)
{
}

void AABBObstacle::Render()
{
	Rgba8 curColor;
	if (m_isRaycast)
	{
		curColor = BRIGHT_AABB_COLOR;
	}
	else
	{
		curColor = NORMAL_AABB_COLOR;
	}
	std::vector<Vertex_PCU> box_verts;

	AddVertsForAABB2D(box_verts,m_AABB2,curColor);
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(box_verts);
}
