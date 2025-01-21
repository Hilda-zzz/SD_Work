#include "Engine/Math/Ray.hpp"

Ray::Ray(Vec2 fwdNormal, Vec2 startPos, float maxLen):m_rayFwdNormal(fwdNormal),m_rayStartPos(startPos),m_rayMaxLength(maxLen)
{
}
