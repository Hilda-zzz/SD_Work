#pragma once
#include "Shape.hpp"
#include "Engine/Math/OBB3.hpp"

class ShapeOBB3D :public Shape
{
public:
	ShapeOBB3D(Vec3 position, OBB3 const& box, Texture* tex, Game3DTestShapes* game, bool isWireFrame);
	void Update(float deltaTime) override;
	void Render() const override;
	Vec3 GetNearestPoint(Vec3 const& referPos) const override;
	RaycastResult3D GetRaycastResult(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist) const;
	void RandomSize() override;
public:
	OBB3 m_box;
	EulerAngles m_euler;
};