#include "LineSegment2.hpp"

LineSegment2::LineSegment2(Vec2 start, Vec2 end)
{
	m_start = start;
	m_end = end;
	m_center = Vec2((m_end.x + m_start.x) * 0.5f, (m_end.y + m_start.y) * 0.5f);
}

LineSegment2::~LineSegment2()
{
}

void LineSegment2::Translate(Vec2 translation)
{
	m_start += translation;
	m_end += translation;
}

void LineSegment2::SetCenter(Vec2 newCenter)
{
	m_start+= newCenter - m_center;
	m_end += newCenter - m_center;
}

void LineSegment2::RotateAboutCenter(float rotationDeltaDegrees)
{
	Vec2 startInOri = m_start - m_center;
	Vec2 endInOri = m_end - m_center;
	startInOri.RotateDegrees(rotationDeltaDegrees);
	endInOri.RotateDegrees(rotationDeltaDegrees);
	m_start = startInOri+m_center;
	m_end = endInOri + m_center;
}
