#include "Game/GameCommon.hpp"


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
	Vertex_PCU vertices[96*3];
	for (int i = 0; i < 96*3; )
	{
		Vec2 InnerFirst = Vec2::MakeFromPolarDegrees(360.f/48 * j, innerRadius)+ori;
		Vec2 OuterFirst = Vec2::MakeFromPolarDegrees(360.f / 48 * j, innerRadius+thickness) + ori;
		Vec2 InnerSecond = Vec2::MakeFromPolarDegrees(360.f / 48 * (j+1), innerRadius) + ori;
		Vec2 OuterSecond = Vec2::MakeFromPolarDegrees(360.f / 48 * (j+1), innerRadius + thickness) + ori;

		vertices[i] = Vertex_PCU(Vec3(InnerFirst.x, InnerFirst.y, 0.f), color, Vec2(0.f, 0.f));
		vertices[i + 1] = Vertex_PCU(Vec3(OuterFirst.x, OuterFirst.y, 0.f), color, Vec2(0.f, 0.f));
		vertices[i + 2] = Vertex_PCU(Vec3(InnerSecond.x, InnerSecond.y, 0.f), color, Vec2(0.f, 0.f));

		vertices[i + 3] = Vertex_PCU(Vec3(InnerSecond.x, InnerSecond.y, 0.f), color, Vec2(0.f, 0.f));
		vertices[i + 4] = Vertex_PCU(Vec3(OuterSecond.x, OuterSecond.y, 0.f), color, Vec2(0.f, 0.f));
		vertices[i + 5] = Vertex_PCU(Vec3(OuterFirst.x, OuterFirst.y, 0.f), color, Vec2(0.f, 0.f));

		i += 6;
		j += 1;
	}
	g_theRenderer->DrawVertexArray(96*3, vertices);
}

void DebugDrawCircle(float radius, Vec2 ori,Rgba8 color)
{
	Vertex_PCU vertices[48];
	int j = 0;
	for (int i = 0; i < 48; )
	{
		Vec2 curPosFirst = Vec2::MakeFromPolarDegrees(22.5f* j, radius)+ori;
		Vec2 curPosSecond = Vec2::MakeFromPolarDegrees(22.5f * (j + 1), radius)+ori;
		vertices[i] = Vertex_PCU(Vec3(ori.x, ori.y, 0.f), color, Vec2(0.f, 0.f));
		vertices[i + 1] = Vertex_PCU(Vec3(curPosFirst.x, curPosFirst.y, 0.f), color, Vec2(0.f, 0.f));
		vertices[i + 2] = Vertex_PCU(Vec3(curPosSecond.x, curPosSecond.y, 0.f), color, Vec2(0.f, 0.f));
		i += 3;
		j += 1;
	}
	g_theRenderer->DrawVertexArray(48, vertices);
}

void DebugDrawHighCircle(float radius, Vec2 ori, Rgba8 color)
{
	Vertex_PCU vertices[96];
	int j = 0;
	for (int i = 0; i < 96; )
	{
		Vec2 curPosFirst = Vec2::MakeFromPolarDegrees(11.25f * j, radius) + ori;
		Vec2 curPosSecond = Vec2::MakeFromPolarDegrees(11.25f * (j + 1), radius) + ori;
		vertices[i] = Vertex_PCU(Vec3(ori.x, ori.y, 0.f), color, Vec2(0.f, 0.f));
		vertices[i + 1] = Vertex_PCU(Vec3(curPosFirst.x, curPosFirst.y, 0.f), color, Vec2(0.f, 0.f));
		vertices[i + 2] = Vertex_PCU(Vec3(curPosSecond.x, curPosSecond.y, 0.f), color, Vec2(0.f, 0.f));
		i += 3;
		j += 1;
	}
	g_theRenderer->DrawVertexArray(96, vertices);
}

void DebugDrawBoxLine(Vec2 botLeft, Vec2 topRight,float width, Rgba8 color)
{
	DebugDrawLine(botLeft, Vec2(topRight.x, botLeft.y),width, color);
	DebugDrawLine(botLeft, Vec2(botLeft.x, topRight.y), width, color);
	DebugDrawLine(topRight, Vec2(botLeft.x, topRight.y), width, color);
	DebugDrawLine(topRight, Vec2(topRight.x, botLeft.y), width, color);
}

void DebugDrawBox(Vec2 botLeft, Vec2 topRight, Rgba8 color)
{
	Vertex_PCU vertices[6];
	vertices[0] = Vertex_PCU(Vec3(botLeft.x, botLeft.y, 0.f), color, Vec2(0.f, 0.f));
	vertices[1] = Vertex_PCU(Vec3(botLeft.x, topRight.y, 0.f), color, Vec2(0.f, 0.f));
	vertices[2] = Vertex_PCU(Vec3(topRight.x, topRight.y, 0.f), color, Vec2(0.f, 0.f));

	vertices[3] = Vertex_PCU(Vec3(botLeft.x, botLeft.y, 0.f), color, Vec2(0.f, 0.f));
	vertices[4] = Vertex_PCU(Vec3(topRight.x, botLeft.y, 0.f), color, Vec2(0.f, 0.f));
	vertices[5] = Vertex_PCU(Vec3(topRight.x, topRight.y, 0.f), color, Vec2(0.f, 0.f));

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(6, vertices);
}

void DebugDrawBox(AABB2 const& bounds, Rgba8 color)
{
	DebugDrawBox(bounds.m_mins, bounds.m_maxs, color);
}
