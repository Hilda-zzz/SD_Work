#include "ShapePlane3D.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Game3DTestShapes.hpp"

ShapePlane3D::ShapePlane3D(Plane3 const& plane, Vec3 position, Texture* tex, Game3DTestShapes* game, bool isWireFrame)
	:Shape(position,tex,game,isWireFrame)
{
	m_plane = plane;
	m_shapeType = ShapeType::TYPE_PLANE;

	AddVertsForPlane3D(m_verts, plane);
}

void ShapePlane3D::Update(float deltaTime)
{
	UNUSED(deltaTime);
}

void ShapePlane3D::Render() const
{
	g_theRenderer->SetModelConstants(Mat44(), m_curColor);

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);

	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->DrawVertexArray(m_verts);
}

Vec3 ShapePlane3D::GetNearestPoint(Vec3 const& referPos) const
{
	return GetNearestPointOnPlane3D(referPos, m_plane);
}

RaycastResult3D ShapePlane3D::GetRaycastResult(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist) const
{
	return RaycastVsPlane3(startPos, fwdNormal, maxDist, m_plane);
}

void ShapePlane3D::RandomSize()
{
	float i_x = m_theGame->m_rng.RollRandomFloatInRange(-5.f, 5.f);
	float i_y = m_theGame->m_rng.RollRandomFloatInRange(-5.f, 5.f);
	float i_z = m_theGame->m_rng.RollRandomFloatInRange(-5.f, 5.f);
	float dist = m_theGame->m_rng.RollRandomFloatInRange(3.f, 8.f);

	Plane3 newPlane = Plane3(Vec3(i_x, i_y, i_z).GetNormalized(), dist);
	m_plane = newPlane;

	m_verts.clear();
	m_verts.reserve(1000);

	AddVertsForPlane3D(m_verts, newPlane);
}
