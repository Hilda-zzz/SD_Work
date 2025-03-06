#include "LineSegObstacle.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/GameCommon.hpp"
#include <Engine/Core/VertexUtils.hpp>

Rgba8 NORMAL_LINE_COLOR = Rgba8(0, 100, 225, 255);
Rgba8 BRIGHT_LINE_COLOR = Rgba8(150, 100, 225, 255);

LineSegObstacle::LineSegObstacle():m_lineSeg(LineSegment2(Vec2(0.f,0.f),Vec2(1.f,1.f)))
{
}

LineSegObstacle::LineSegObstacle(LineSegment2& lineSeg):m_lineSeg(lineSeg),m_isRaycast(false)
{
}

void LineSegObstacle::Render()
{
	Rgba8 curColor;
	if (m_isRaycast)
	{
		curColor = BRIGHT_LINE_COLOR;
	}
	else
	{
		curColor = NORMAL_LINE_COLOR;
	}
	std::vector<Vertex_PCU> line_verts;
	
	AddVertsForLinSegment2D(line_verts, m_lineSeg.m_start, m_lineSeg.m_end, 2.f, curColor);
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(line_verts);
}
