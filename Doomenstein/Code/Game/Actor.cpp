#include "Actor.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <Engine/Renderer/Renderer.hpp>

extern Renderer* g_theRenderer;

Actor::Actor(Vec3 const& positon, EulerAngles const& orientaion,
	Rgba8 const& color, bool isStatic, ZCylinder const& collider)
	:m_position(positon),m_orientation(orientaion),m_color(color),m_isStatic(isStatic),m_physicCollider(collider)
{
	AddVertsForCylinder3D(m_vertexs, m_physicCollider.m_center-Vec3(0.f,0.f, m_physicCollider.m_halfHeight),
		m_physicCollider.m_center + Vec3(0.f, 0.f, m_physicCollider.m_halfHeight), 
		m_physicCollider.m_radius,m_color,AABB2::ZERO_TO_ONE,16);
}

void Actor::Update()
{
}

void Actor::Render() const
{
	g_theRenderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_BACK);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(nullptr);
	Mat44 modelMatt = GetModelMat();
	g_theRenderer->SetModelConstants(modelMatt, Rgba8::GREEN);
	g_theRenderer->DrawVertexArray(m_vertexs);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatt, m_color);
	g_theRenderer->DrawVertexArray(m_vertexs);
}

Mat44 Actor::GetModelMat() const
{
	Mat44 translation = Mat44::MakeTranslation3D(m_position);
	Mat44 orientation = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	translation.Append(orientation);
	return translation;
}
