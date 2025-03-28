#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Mat44.hpp"	
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/RaycastUtils.hpp"
class Texture;
class Clock;
class Game3DTestShapes;
enum class ShapeType
{
	TYPE_SHPERE,
	TYPE_BOX3D,
	TYPE_ZCYLINDER,
	COUNT
};

class Shape
{
public:
	Shape(Vec3 position, Texture* tex, Game3DTestShapes* game,bool isWireFrame=false):m_center(position),m_texture(tex),m_theGame(game),m_isWireFrame(isWireFrame) {};
	virtual ~Shape() {};
	virtual void Update(float deltaTime);
	virtual void Render() const = 0;
	virtual Vec3 GetNearestPoint(Vec3 const& referPos) const = 0;
	virtual RaycastResult3D GetRaycastResult(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist) const = 0;
	virtual void RandomSize() = 0;

	Mat44 GetModelToWorldTransform() const
	{
		Mat44 modelToWorld = Mat44::MakeTranslation3D(m_center);
		modelToWorld.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
		return modelToWorld;
	}
public:
	Game3DTestShapes* m_theGame = nullptr;
	Clock* m_clock = nullptr;;
	Vec3 m_center = Vec3(0.f,0.f,0.f);
	EulerAngles m_orientation=EulerAngles(0.f,0.f,0.f);
	Rgba8 m_color = Rgba8::WHITE;
	Rgba8 m_curColor= Rgba8::WHITE;
	std::vector<Vertex_PCU> m_verts;
	Texture* m_texture = nullptr;
	bool m_isRayHit = false;
	bool m_isNearest=false;
	bool m_isGrab = false;
	bool m_isOverlap = false;
	Vec3 m_nearestPoint;
	ShapeType m_shapeType=ShapeType::TYPE_SHPERE;
	bool m_isWireFrame = false;
};