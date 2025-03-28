#include "ZCylinder.hpp"
#include <Engine/Core/VertexUtils.hpp>
#include <Engine/Math/MathUtils.hpp>
#include "Game/Game3DTestShapes.hpp"

ZCylinder::ZCylinder(Vec3 position, float halfHeight, float radius, Texture* tex, Game3DTestShapes* game, bool isWireFrame)
	:Shape(position, tex,game,isWireFrame)
{
	m_radius = radius;
	m_halfHeight = halfHeight;
	if (m_isWireFrame)
	{
		AddVertsForCylinder3DWireFrame(m_verts, Vec3(0.f, 0.f, -halfHeight), Vec3(0.f, 0.f, halfHeight), radius, m_color, AABB2::ZERO_TO_ONE, 32);
	}
	else
	{
		AddVertsForCylinder3D(m_verts, Vec3(0.f, 0.f, -halfHeight), Vec3(0.f, 0.f, halfHeight), radius, m_color, AABB2::ZERO_TO_ONE, 32);
	}
	m_shapeType = ShapeType::TYPE_ZCYLINDER;
}

void ZCylinder::Update(float deltaTime)
{
	Shape::Update(deltaTime);
}

void ZCylinder::Render() const
{
	g_theRenderer->SetModelConstants(GetModelToWorldTransform(), m_curColor);

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);

	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->DrawVertexArray(m_verts);
}

Vec3 ZCylinder::GetNearestPoint(Vec3 const& referPos) const
{
	return GetNearestPointOnZCylinder3D(referPos,m_center,m_radius,m_halfHeight);
}

RaycastResult3D ZCylinder::GetRaycastResult(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist) const
{
	return RaycastVsZCylinder3D(startPos, fwdNormal, maxDist, m_center, m_radius,m_halfHeight);
}

void ZCylinder::RandomSize()
{
	float newRadius= m_theGame->m_rng.RollRandomFloatInRange(0.5f, 1.f);
	float newHalfHeight= m_theGame->m_rng.RollRandomFloatInRange(0.3f, 1.5f);
	int sliceNum = m_theGame->m_rng.RollRandomIntInRange(8, 32);

	m_radius = newRadius;
	m_halfHeight = newHalfHeight;


	m_verts.clear();
	m_verts.reserve(500);
	if (m_isWireFrame)
	{
		AddVertsForCylinder3DWireFrame(m_verts, Vec3(0.f, 0.f, -newHalfHeight), Vec3(0.f, 0.f, newHalfHeight), newRadius, m_color, AABB2::ZERO_TO_ONE, sliceNum);
	}
	else
	{
		AddVertsForCylinder3D(m_verts, Vec3(0.f, 0.f, -newHalfHeight), Vec3(0.f, 0.f, newHalfHeight), newRadius, m_color, AABB2::ZERO_TO_ONE, sliceNum);
	}
}
