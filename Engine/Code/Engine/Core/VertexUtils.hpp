#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
#include "Engine/Math/Triangle2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Vertex_PCUTBN.hpp"

//#include "Game/ShapePlane3D.hpp"

struct Vec2;
struct Rgba8;
class OBB2;
struct AABB2;
struct AABB3;
class Mat44;
class Plane3;
class OBB3;

void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float uniformScaleXY,
	float rotationDegreesAboutZ, Vec2 const& translationXY);
void TransformVertexArrayXY3D(int numVerts, std::vector<Vertex_PCU>& verts, float uniformScaleXY,
	float rotationDegreesAboutZ, Vec2 const& translationXY);
void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, const Mat44& transform);

//void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Capsule2 const& capsule, Rgba8 const& color);
void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius,Rgba8 const& color);
void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color);
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds,Rgba8 const& color);
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color,Vec2 const& uv_mins, Vec2 const& uv_maxs);
void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts,OBB2 const&box, Rgba8 const& color);
void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color, Vec2 const& uv_mins, Vec2 const& uv_maxs);
void AddVertsForTriangle2D(std::vector<Vertex_PCU>& verts, Triangle2 const& triangle, Rgba8 const& color);
//void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, Vec2 const& center,Vec2 const& iBasis,Vec2 dimensions, Rgba8 const& color);
void AddVertsForLinSegment2D(std::vector<Vertex_PCU>& verts, Vec2 const& start,Vec2 const& end,float thickness, Rgba8 const& color);
//void AddVertsForLinSegment2D(std::vector<Vertex_PCU>& verts,LineSegment2 const& lineSegment,float thickness , Rgba8 const& color);
void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 const& color);

void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts,
	const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft,
	const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes,
	const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft,
	const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D_WithTBN(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes,
	const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft,
	const Rgba8& color, const AABB2& UVs);
void AddVertsForQuad3D_WithTBN(std::vector<Vertex_PCUTBN>& verts,
	const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft,
	const Rgba8& color, const AABB2& UVs);

void AddVertsForRoundedQuad3D(std::vector<Vertex_PCUTBN>& vertexes, const Vec3& bottomLeft, const Vec3& bottomRight, 
	const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs);;

void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, 
	const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius,
	const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE, int numSlices = 32, int numStacks = 16);
void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius,
	const Rgba8 color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE, int numSlices = 8);
void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius,
	const Rgba8 color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE, int numSlices = 8);
void AddVertsForOBB3D(std::vector<Vertex_PCU>& verts, const OBB3& box,
	const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForAABB3DWireFrame(std::vector<Vertex_PCU>& verts, const AABB3& bounds,
	const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForSphere3DWireFrame(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius,
	const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE, int numSlices = 32, int numStacks = 16);
void AddVertsForCylinder3DWireFrame(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius,
	const Rgba8 color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE, int numSlices = 8);
void AddVertsForOBB3DWireFrame(std::vector<Vertex_PCU>& verts, const OBB3& box,
	const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForPlane3D(std::vector<Vertex_PCU>& verts, Plane3 const& plane);

AABB2 GetVertexBounds2D(const std::vector<Vertex_PCU>& verts);

void AddVertsForCubeSkyBox(std::vector<Vertex_PCU>& verts, const Vec3& center, float size, const Rgba8& color);