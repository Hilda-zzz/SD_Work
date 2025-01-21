#include "DiscObstacle.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>
#include <Engine/Core/Vertex_PCU.hpp>
#include <Engine/Core/VertexUtils.hpp>
#include "Engine/Renderer/Renderer.hpp"
#include "GameCommon.hpp"
#include <Engine/Math/RaycastUtils.hpp>
Rgba8 NORMAL_DISC_COLOR = Rgba8(0, 100, 225, 255);
Rgba8 BRIGHT_DISC_COLOR = Rgba8(150, 100, 225, 255);

DiscObstacle::DiscObstacle(Vec2 center, float radius):m_center(center),m_radius(radius)
{

}

void DiscObstacle::Render()
{
	Rgba8 curColor;
	if (m_isRaycast)
	{
		curColor = BRIGHT_DISC_COLOR;
	}
	else
	{
		curColor = NORMAL_DISC_COLOR;
	}
	std::vector<Vertex_PCU> disc_verts;
	AddVertsForDisc2D(disc_verts, m_center, m_radius, curColor);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(disc_verts);
}

