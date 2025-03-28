#pragma once
#include "Game/Shape.hpp"
#include "Game/GameCommon.hpp"

class Box3D :public Shape
{
public:
	Box3D(Vec3 position,AABB3 const& box, Texture* tex, Game3DTestShapes* game, bool isWireFrame);
	void Update(float deltaTime) override;
	void Render() const override;
	Vec3 GetNearestPoint(Vec3 const& referPos) const override;
	RaycastResult3D GetRaycastResult(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist) const;
	void RandomSize() override;
public:
	AABB3 m_box;
};