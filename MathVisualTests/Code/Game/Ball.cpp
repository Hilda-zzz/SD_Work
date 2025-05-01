#include "Ball.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Game/GameCommon.hpp"

void Ball::Update(float deltaTime)
{
	//gravity
	m_acceleration += Vec2(0.f,-800.f);

	m_velocity += m_acceleration * deltaTime;
	m_pos += m_velocity * deltaTime;

	//trans above the top
	if (m_pos.y < -m_radius)
	{
		m_pos.y = SCREEN_SIZE_Y * 1.1f + m_radius;
	}

	m_acceleration = Vec2::ZERO;
}

//void Ball::Render() const
//{
//	Mat44 modelToWorld = Mat44::MakeTranslation2D(m_pos);
//	g_theRenderer->SetModelConstants(modelToWorld);
//	g_theRenderer->BindShader(nullptr);
//	g_theRenderer->BindTexture(nullptr);
//	g_theRenderer->DrawVertexArray(m_verts);
//}
