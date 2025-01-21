#include "RaycastAtrrow.hpp"

RaycastArrow::RaycastArrow(Vec2 startPos, Vec2 direction, float maxLen):m_startPos(startPos),m_direction(direction),m_maxLen(maxLen)
{
	m_endPos_whole = m_startPos + m_direction.GetNormalized() * m_maxLen;
}
