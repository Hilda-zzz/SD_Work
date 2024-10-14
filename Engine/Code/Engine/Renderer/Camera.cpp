
#include <Engine/Renderer/Camera.hpp>
#include <Game/GameCommon.hpp>

Camera::Camera():m_bottomLeft(Vec2{0.f,0.f}), m_topRight(Vec2{ 0.f,0.f })
{
}

void Camera::SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight)
{
	m_bottomLeft = bottomLeft;
	m_topRight = topRight;
}

Vec2 Camera::GetOrthoBottomLeft() const
{
	return m_bottomLeft;
}

Vec2 Camera::GetOrthoTopRight() const
{
	return m_topRight;
}

void Camera::Translate2DFromPosition(const Vec2& translation2D, Vec2 oriPos_bl, Vec2 oriPos_tr)
{
	m_bottomLeft = oriPos_bl+translation2D;
	m_topRight = oriPos_tr+translation2D;
}

//void Camera::Translate2DToFollowedObj(const Vec2& aimPos)
//{
//	Vec2 blTrans = Vec2(-WORLD_CENTER_X,-WORLD_CENTER_Y);
//	Vec2 trTrans = Vec2(WORLD_CENTER_X, WORLD_CENTER_Y);
//	m_bottomLeft = aimPos + blTrans;
//	m_topRight = aimPos + trTrans;
//}
