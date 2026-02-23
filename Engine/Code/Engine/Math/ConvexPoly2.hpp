#pragma once
#include "Engine/Math/Vec2.hpp"
#include <vector>

class RandomNumberGenerator;

class ConvexPoly2
{
public:
	ConvexPoly2();
	explicit ConvexPoly2(std::vector<Vec2> const& vertices);
	~ConvexPoly2();

	void GenerateRandomConvex(int numVertices, Vec2 const& center, float radius, RandomNumberGenerator& rng);

	// Create specific shapes
	void MakeRegularPolygon(int numSides, Vec2 const& center, float radius);
	void MakeBox(Vec2 const& mins, Vec2 const& maxs);
	void MakeBoxCentered(Vec2 const& center, float width, float height);

	// Set from existing vertices (assumes vertices are already in CCW order)
	void SetFromVertices(std::vector<Vec2> const& vertices);

	//-----------------------------------------------------------------------------------------------
	// Transformations
	//-----------------------------------------------------------------------------------------------
	// Transform around an arbitrary pivot point
	void RotateAroundPoint(Vec2 const& pivot, float degrees);
	void ScaleAroundPoint(Vec2 const& pivot, float uniformScale);
	void Translate(Vec2 const& offset);

	// Combined transform for efficiency
	void TransformAroundPoint(Vec2 const& pivot, float rotationDegrees, float uniformScale);

	// Get transformed copy without modifying this polygon
	ConvexPoly2 GetTransformed(Vec2 const& pivot, float rotationDegrees, float uniformScale) const;

	//-----------------------------------------------------------------------------------------------
	// Geometric Queries
	//-----------------------------------------------------------------------------------------------
	// Get center (average of all vertices)
	Vec2 GetCenter() const;

	// Get smallest circle that contains all vertices
	void CalculateBoundingRadiusAndCenter();
	float GetBoundingRadius() const;
	Vec2 GetBoundingDiscCenter() const { return GetCenter(); }

	// Get axis-aligned bounding box
	void GetBounds(Vec2& out_mins, Vec2& out_maxs) const;

	// Check if polygon contains a point (inside test using winding number or cross products)
	bool ContainsPoint(Vec2 const& point) const;

	//-----------------------------------------------------------------------------------------------
	// Accessors
	//-----------------------------------------------------------------------------------------------
	std::vector<Vec2> const& GetVertices() const { return m_vertices; }
	std::vector<Vec2>& GetVertices() { return m_vertices; }
	int GetVertexCount() const { return (int)m_vertices.size(); }
	Vec2 GetVertex(int index) const;

	// Get edge (returns two consecutive vertices)
	void GetEdge(int edgeIndex, Vec2& out_start, Vec2& out_end) const;
	int GetEdgeCount() const { return GetVertexCount(); }

	//-----------------------------------------------------------------------------------------------
	// Validation
	//-----------------------------------------------------------------------------------------------
	// Check if vertices form a valid convex polygon (for debugging)
	bool IsConvex() const;
	bool IsValid() const;		// At least 3 vertices, all finite, forms convex shape

private:
	//-----------------------------------------------------------------------------------------------
	// Internal helpers
	//-----------------------------------------------------------------------------------------------
	// Ensure vertices are in CCW order
	void EnsureCCWWinding();

	// Calculate signed area (positive = CCW, negative = CW)
	float GetSignedArea() const;

private:
	std::vector<Vec2> m_vertices;		// Vertices in CCW order
};