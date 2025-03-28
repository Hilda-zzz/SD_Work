#pragma once
#include "Game/Shape.hpp"
#include "Game/GameCommon.hpp"

class ZCylinder :public Shape
{
public:
	ZCylinder(Vec3 position, float halfHeight,float radius, Texture* tex, Game3DTestShapes* game, bool isWireFrame);
	void Update(float deltaTime) override;
	void Render() const override;
	Vec3 GetNearestPoint(Vec3 const& referPos) const override;
	RaycastResult3D GetRaycastResult(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist) const;
	void RandomSize() override;
public:
	float m_halfHeight;
	float m_radius;
};