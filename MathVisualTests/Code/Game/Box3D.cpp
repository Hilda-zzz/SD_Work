#include "Box3D.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <Engine/Math/MathUtils.hpp>
#include "Game/Game3DTestShapes.hpp"

Box3D::Box3D(Vec3 position,AABB3 const& box, Texture* tex, Game3DTestShapes* game,bool isWireFrame)
	:Shape(position, tex, game, isWireFrame)
{
	m_box = box;
	if (isWireFrame)
	{
		AddVertsForAABB3DWireFrame(m_verts, m_box);
	}
	else
	{
		AddVertsForAABB3D(m_verts, m_box);
	}
	m_shapeType = ShapeType::TYPE_BOX3D;
}

void Box3D::Update(float deltaTime)
{
	Shape::Update(deltaTime);
}

void Box3D::Render() const
{
	g_theRenderer->SetModelConstants(GetModelToWorldTransform(), m_curColor);

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);

	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->DrawVertexArray(m_verts);
}

Vec3 Box3D::GetNearestPoint(Vec3 const& referPos) const
{
	return GetNearestPointOnAABB3D(referPos, AABB3(m_box.m_mins+m_center,m_box.m_maxs+m_center));
}

RaycastResult3D Box3D::GetRaycastResult(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist) const
{
	AABB3 realPosBox = AABB3(m_box.m_mins + m_center, m_box.m_maxs + m_center);
	return RaycastVsAABB3D(startPos, fwdNormal, maxDist, realPosBox);
}

void Box3D::RandomSize()
{
	float xHalfLen = m_theGame->m_rng.RollRandomFloatInRange(0.5f, 1.5f);
	float yHalfLen = m_theGame->m_rng.RollRandomFloatInRange(0.5f, 1.5f);
	float zHalfLen = m_theGame->m_rng.RollRandomFloatInRange(0.5f, 1.5f);
	AABB3 newBox = AABB3(Vec3(-xHalfLen, -yHalfLen, -zHalfLen), Vec3(xHalfLen, yHalfLen, zHalfLen));

	m_box = newBox;

	m_verts.clear();
	m_verts.reserve(00);
	if (m_isWireFrame)
	{
		AddVertsForAABB3DWireFrame(m_verts, newBox);
	}
	else
	{
		AddVertsForAABB3D(m_verts, newBox);
	}
}
