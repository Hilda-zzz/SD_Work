#include "ShapeOBB3D.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Shape.hpp"
#include "Game/Game3DTestShapes.hpp"

ShapeOBB3D::ShapeOBB3D(Vec3 position, OBB3 const& box, Texture* tex, Game3DTestShapes* game, bool isWireFrame)
	:Shape(position, tex, game, isWireFrame)
{
	m_box = OBB3(box.m_iBasis, box.m_halfDimensions,Vec3(0.f,0.f,0.f));

	if (!isWireFrame)
	{
		AddVertsForOBB3D(m_verts, box);
	}
	else 
	{
		AddVertsForOBB3DWireFrame(m_verts, box);
	}
	
	AddVertsForCylinder3D(m_verts, box.m_center, box.m_center + box.m_iBasis, 0.01f, Rgba8::RED);
	AddVertsForCylinder3D(m_verts, box.m_center, box.m_center + box.m_jBasis, 0.01f, Rgba8::GREEN);
	AddVertsForCylinder3D(m_verts, box.m_center, box.m_center + box.m_kBasis, 0.01f, Rgba8::BLUE);

	m_shapeType = ShapeType::TYPE_OBB3D;
}

void ShapeOBB3D::Update(float deltaTime)
{
	Shape::Update(deltaTime);

	Vec3 i, j, k;
	m_euler.GetAsVectors_IFwd_JLeft_KUp(i, j, k);
	m_box = OBB3(i,j,k, m_box.m_halfDimensions, Vec3(0.f, 0.f, 0.f));

	m_verts.clear();
	if (!m_isWireFrame)
	{
		AddVertsForOBB3D(m_verts, m_box);
	}
	else
	{
		AddVertsForOBB3DWireFrame(m_verts, m_box);
	}

	AddVertsForCylinder3D(m_verts, m_box.m_center, m_box.m_center + m_box.m_iBasis, 0.01f, Rgba8::RED);
	AddVertsForCylinder3D(m_verts, m_box.m_center, m_box.m_center + m_box.m_jBasis, 0.01f, Rgba8::GREEN);
	AddVertsForCylinder3D(m_verts, m_box.m_center, m_box.m_center + m_box.m_kBasis, 0.01f, Rgba8::BLUE);

}

void ShapeOBB3D::Render() const
{
	g_theRenderer->SetModelConstants(GetModelToWorldTransform(), m_curColor);

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);

	if (!m_isWireFrame)
	{
		g_theRenderer->BindTexture(m_texture);
	}
	else
	{
		g_theRenderer->BindTexture(nullptr);
	}
	g_theRenderer->DrawVertexArray(m_verts);
}

Vec3 ShapeOBB3D::GetNearestPoint(Vec3 const& referPos) const
{
	OBB3 realBox = OBB3(m_box.m_iBasis, m_box.m_halfDimensions, m_center);
	return GetNearestPointOnOBB3D(referPos, realBox);
}

RaycastResult3D ShapeOBB3D::GetRaycastResult(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist) const
{
	OBB3 realBox = OBB3(m_box.m_iBasis, m_box.m_halfDimensions, m_center);
	return RaycastVsOBB3(startPos, fwdNormal, maxDist, realBox);
}

void ShapeOBB3D::RandomSize()
{
	float xHalfLen = m_theGame->m_rng.RollRandomFloatInRange(0.5f, 1.5f);
	float yHalfLen = m_theGame->m_rng.RollRandomFloatInRange(0.5f, 1.5f);
	float zHalfLen = m_theGame->m_rng.RollRandomFloatInRange(0.5f, 1.5f);
	m_euler.m_yawDegrees = (float)m_theGame->m_rng.RollRandomIntInRange(-36, 36) * 10.f;
	m_euler.m_pitchDegrees= (float)m_theGame->m_rng.RollRandomIntInRange(-36, 36) * 10.f;
	m_euler.m_rollDegrees = (float)m_theGame->m_rng.RollRandomIntInRange(-36, 36) * 10.f;
	Vec3 i, j, k;
	m_euler.GetAsVectors_IFwd_JLeft_KUp(i, j, k);
	OBB3 newBox = OBB3(i,j,k,Vec3(xHalfLen, yHalfLen, zHalfLen),Vec3(0.f,0.f,0.f));
	m_box = newBox;

	m_verts.clear();
	m_verts.reserve(100);
	if (m_isWireFrame)
	{
		AddVertsForOBB3DWireFrame(m_verts, newBox);
	}
	else
	{
		AddVertsForOBB3D(m_verts, newBox);
	}

	AddVertsForCylinder3D(m_verts, newBox.m_center, newBox.m_center + newBox.m_iBasis, 0.01f, Rgba8::RED);
	AddVertsForCylinder3D(m_verts, newBox.m_center, newBox.m_center + newBox.m_jBasis, 0.01f, Rgba8::GREEN);
	AddVertsForCylinder3D(m_verts, newBox.m_center, newBox.m_center + newBox.m_kBasis, 0.01f, Rgba8::BLUE);
}
