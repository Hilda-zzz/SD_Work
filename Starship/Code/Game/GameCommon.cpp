#include <Game/GameCommon.hpp>
#include <Engine/Core/Vertex_PCU.hpp>
#include "Engine/Renderer/Renderer.hpp"

extern Renderer*g_theRenderer;
void DebugDrawLine(Vec2 const& start, Vec2 const& end, float width,Rgba8 color)
{
	Vec2 step_fwd = end - start;
	step_fwd = step_fwd.GetNormalized();
	Vec2 step_left = step_fwd.GetRotated90Degrees();
	float halfWidth = width / 2.f;
	step_fwd *= halfWidth;
	step_left *= halfWidth;
	Vec2 SL = start - step_fwd + step_left;
	Vec2 SR = start - step_fwd - step_left;
	Vec2 EL = end + step_fwd + step_left;
	Vec2 ER = end + step_fwd - step_left;

	Vertex_PCU vertices[6];
	vertices[0] = Vertex_PCU(Vec3(SL.x,SL.y, 0.f), color, Vec2(0.f, 0.f));
	vertices[1] = Vertex_PCU(Vec3(SR.x, SR.y, 0.f), color, Vec2(0.f, 0.f));
	vertices[2] = Vertex_PCU(Vec3(EL.x, EL.y, 0.f), color, Vec2(0.f, 0.f));

	vertices[3] = Vertex_PCU(Vec3(EL.x, EL.y, 0.f), color, Vec2(0.f, 0.f));
	vertices[4] = Vertex_PCU(Vec3(ER.x, ER.y, 0.f), color, Vec2(0.f, 0.f));
	vertices[5] = Vertex_PCU(Vec3(SR.x, SR.y, 0.f), color, Vec2(0.f, 0.f));
	g_theRenderer->DrawVertexArray(6, vertices);
}

void DebugDrawRing(float thickness,float innerRadius,Rgba8 color,Vec2 ori)
{
	int j = 0;
	Vertex_PCU vertices[96];
	for (int i = 0; i < 96; )
	{
		Vec2 InnerFirst = Vec2::MakeFromPolarDegrees(360.f/16 * j, innerRadius)+ori;
		Vec2 OuterFirst = Vec2::MakeFromPolarDegrees(360.f / 16 * j, innerRadius+thickness) + ori;
		Vec2 InnerSecond = Vec2::MakeFromPolarDegrees(360.f / 16 * (j+1), innerRadius) + ori;
		Vec2 OuterSecond = Vec2::MakeFromPolarDegrees(360.f / 16 * (j+1), innerRadius + thickness) + ori;

		vertices[i] = Vertex_PCU(Vec3(InnerFirst.x, InnerFirst.y, 0.f), color, Vec2(0.f, 0.f));
		vertices[i + 1] = Vertex_PCU(Vec3(OuterFirst.x, OuterFirst.y, 0.f), color, Vec2(0.f, 0.f));
		vertices[i + 2] = Vertex_PCU(Vec3(InnerSecond.x, InnerSecond.y, 0.f), color, Vec2(0.f, 0.f));

		vertices[i + 3] = Vertex_PCU(Vec3(InnerSecond.x, InnerSecond.y, 0.f), color, Vec2(0.f, 0.f));
		vertices[i + 4] = Vertex_PCU(Vec3(OuterSecond.x, OuterSecond.y, 0.f), color, Vec2(0.f, 0.f));
		vertices[i + 5] = Vertex_PCU(Vec3(OuterFirst.x, OuterFirst.y, 0.f), color, Vec2(0.f, 0.f));

		i += 6;
		j += 1;
	}
	g_theRenderer->DrawVertexArray(96, vertices);
}