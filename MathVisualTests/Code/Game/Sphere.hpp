#pragma once
#include "Game/Shape.hpp"

class Sphere :public Shape
{
public:
	Sphere(Vec3 position,float radius,Texture* tex, Game3DTestShapes* game, bool isWireFrame);
	void Update(float deltaTime) override;
	void Render() const override;
	void RandomSize() override;
	Vec3 GetNearestPoint(Vec3 const& referPos) const override;
	RaycastResult3D GetRaycastResult(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist) const;
public:
	float m_radius = 0.f;
};