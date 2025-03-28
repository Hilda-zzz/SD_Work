#include "Sphere.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/GameCommon.hpp"
#include <Engine/Math/MathUtils.hpp>
#include "Game/Game3DTestShapes.hpp"
Sphere::Sphere(Vec3 position, float radius, Texture* tex, Game3DTestShapes* game, bool isWireFrame)
	:Shape(position,tex,game, isWireFrame)
{
	m_radius = radius;
	if (m_isWireFrame)
	{
		AddVertsForSphere3DWireFrame(m_verts, Vec3(0.f, 0.f, 0.f), m_radius);
	}
	else
	{
		AddVertsForSphere3D(m_verts, Vec3(0.f, 0.f, 0.f), m_radius);
	}
	
	m_shapeType = ShapeType::TYPE_SHPERE;
}

void Sphere::Update(float deltaTime)
{
	Shape::Update(deltaTime);
}

void Sphere::Render() const
{
	g_theRenderer->SetModelConstants(GetModelToWorldTransform(), m_curColor);

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);

	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->DrawVertexArray(m_verts);
}

void Sphere::RandomSize()
{
	float newRadius = m_theGame->m_rng.RollRandomFloatInRange(0.5f, 1.f);
	int sliceNum = m_theGame->m_rng.RollRandomIntInRange(4, 32);
	int stackNum = m_theGame->m_rng.RollRandomIntInRange(4, 32);

	m_radius = newRadius;

	m_verts.clear();
	m_verts.reserve(500);
	if (m_isWireFrame)
	{
		AddVertsForSphere3DWireFrame(m_verts, Vec3(0.f, 0.f, 0.f), newRadius,m_color,AABB2::ZERO_TO_ONE,sliceNum,stackNum);
	}
	else
	{
		AddVertsForSphere3D(m_verts, Vec3(0.f, 0.f, 0.f), newRadius, m_color, AABB2::ZERO_TO_ONE, sliceNum, stackNum);
	}
}

Vec3 Sphere::GetNearestPoint(Vec3 const& referPos) const
{
	return GetNearestPointOnSphere3D(referPos,m_center, m_radius);
}

RaycastResult3D Sphere::GetRaycastResult(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist) const
{
	return RaycastVsSphere3D(startPos, fwdNormal, maxDist, m_center, m_radius);
}
