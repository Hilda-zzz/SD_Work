#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
#include "Engine/Math/Triangle2.hpp"
struct Vec2;
struct Rgba8;
class OBB2;
struct AABB2;

void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float uniformScaleXY,
	float rotationDegreesAboutZ, Vec2 const& translationXY);
void TransformVertexArrayXY3D(int numVerts, std::vector<Vertex_PCU>& verts, float uniformScaleXY,
	float rotationDegreesAboutZ, Vec2 const& translationXY);

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