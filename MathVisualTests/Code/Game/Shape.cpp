#include "Shape.hpp"
#include <Engine/Math/MathUtils.hpp>
#include "Engine/Core/Clock.hpp"
#include "Game/Game3DTestShapes.hpp"
void Shape::Update(float deltaTime)
{
	UNUSED(deltaTime);
	if (m_isRayHit)
	{
		m_color = Rgba8::HILDA;
	}
	else
	{
		m_color = Rgba8::WHITE;
	}

	if (m_isGrab)
	{
		m_color = Rgba8::RED;
	}

	m_curColor = m_color;
	if (m_isOverlap)
	{
		float colorRatio = CosDegrees(300.f * (float)m_theGame->m_gameClock->GetTotalSeconds());
		colorRatio =RangeMapClamped(colorRatio, -1.f, 1.f, 0.2f, 1.f);
		m_curColor = Rgba8((unsigned char)(colorRatio *m_color.r), (unsigned char)(colorRatio * m_color.g), (unsigned char)(colorRatio * m_color.b), 255);
	}
}
