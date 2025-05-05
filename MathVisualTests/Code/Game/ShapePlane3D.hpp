#pragma once
#include "Game/Shape.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/Plane3.hpp"

class ShapePlane3D :public Shape
{
public:
	ShapePlane3D(Plane3 const& plane, Vec3 position, Texture* tex = nullptr, Game3DTestShapes* game = nullptr, bool isWireFrame = false);
	void Update(float deltaTime) override;
	void Render() const override;
	Vec3 GetNearestPoint(Vec3 const& referPos) const override;
	RaycastResult3D GetRaycastResult(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist) const override;
	void RandomSize() override;
public:
	Plane3 m_plane;
};