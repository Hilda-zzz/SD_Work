#pragma once

#include <Engine/Math/Vec2.hpp>

class Camera
{
public:
	Camera();
	void SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight);
	Vec2 GetOrthoBottomLeft() const;
	Vec2 GetOrthoTopRight() const;
	void Translate2DFromPosition(const Vec2& translation2D,Vec2 oriPos_lb,Vec2 oriPos_tr);
	//void Translate2DToFollowedObj(const Vec2&  newPos);
private:
	Vec2 m_bottomLeft;
	Vec2 m_topRight;
};