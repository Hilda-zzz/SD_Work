#pragma once
#include <vector>
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "RandomNumberGenerator.hpp"

class ConvexShape2D
{
public:
	ConvexShape2D();
	~ConvexShape2D();

	void GenerateRandomConvex(int numVertices, Vec2 const& center, float minRadius, float maxRadius, RandomNumberGenerator& rng);

	// Create specific shapes
	void MakeRegularPolygon(int numSides, Vec2 const& center, float radius);
	void MakeBox(Vec2 const& center, float width, float height);
	void MakeFromVertices(std::vector<Vec2> const& vertices);	// Assumes vertices are already in CCW order

	//-----------------------------------------------------------------------------------------------
	// Spatial Queries
	//-----------------------------------------------------------------------------------------------
	bool ContainsPoint(Vec2 const& point) const;
	RaycastResult2D Raycast(Vec2 const& rayStart, Vec2 const& rayEnd, bool generateDebugData = false) const;

	// Bounding volume queries
	float GetBoundingRadius() const { return m_boundingRadius; }
	Vec2 GetCenter() const { return m_center; }
	bool DoesRayIntersectBoundingDisc(Vec2 const& rayStart, Vec2 const& rayEnd) const;

	//-----------------------------------------------------------------------------------------------
	// Transformations
	//-----------------------------------------------------------------------------------------------
	// Transform the shape around a specified pivot point
	void RotateAroundPoint(Vec2 const& pivot, float degrees);
	void ScaleAroundPoint(Vec2 const& pivot, float uniformScale);
	void Translate(Vec2 const& offset);

	// Combined transform for efficiency
	void TransformAroundPoint(Vec2 const& pivot, float rotationDegrees, float uniformScale);

	//-----------------------------------------------------------------------------------------------
	// Accessors / Mutators
	//-----------------------------------------------------------------------------------------------
	std::vector<Vec2> const& GetVertices() const { return m_vertices; }
	std::vector<Plane2D> const& GetPlanes() const { return m_planes; }
	int GetVertexCount() const { return (int)m_vertices.size(); }

	void SetFillColor(Rgba8 const& color) { m_fillColor = color; }
	void SetEdgeColor(Rgba8 const& color) { m_edgeColor = color; }
	Rgba8 GetFillColor() const { return m_fillColor; }
	Rgba8 GetEdgeColor() const { return m_edgeColor; }

	void SetHighlight(bool isHighlighted) { m_isHighlighted = isHighlighted; }
	bool IsHighlighted() const { return m_isHighlighted; }

	//-----------------------------------------------------------------------------------------------
	// Rendering
	//-----------------------------------------------------------------------------------------------
	void RenderFill() const;								// Draw filled interior
	void RenderEdges(float thickness = 2.f) const;			// Draw thick outlined edges
	void RenderFull(float edgeThickness = 2.f) const;		// Draw both fill and edges
	void RenderBoundingDisc() const;						// Debug draw bounding circle

	// Debug rendering for single-shape raycast visualization
	void RenderRaycastDebug(RaycastResult2D const& result) const;

private:
	//-----------------------------------------------------------------------------------------------
	// Internal helper methods
	//-----------------------------------------------------------------------------------------------
	void UpdatePlanesFromVertices();		// Regenerate planes after vertices change
	void UpdateBoundingDisc();				// Recalculate center and radius after vertices change
	void UpdateDerivedData();				// Update both planes and bounding disc

	// Validate that vertices form a valid convex polygon (for debugging)
	bool IsConvex() const;

private:
	//-----------------------------------------------------------------------------------------------
	// Core geometry data
	//-----------------------------------------------------------------------------------------------
	std::vector<Vec2> m_vertices;			// Vertices in CCW order
	std::vector<Plane2D> m_planes;			// One plane per edge (same count as vertices)

	//-----------------------------------------------------------------------------------------------
	// Bounding volume (for early rejection)
	//-----------------------------------------------------------------------------------------------
	Vec2 m_center = Vec2::ZERO;				// Geometric center (average of vertices)
	float m_boundingRadius = 0.f;			// Radius of smallest circle containing all vertices

	//-----------------------------------------------------------------------------------------------
	// Appearance
	//-----------------------------------------------------------------------------------------------
	Rgba8 m_fillColor = Rgba8(100, 100, 150, 128);		// Default semi-transparent fill
	Rgba8 m_edgeColor = Rgba8(255, 255, 255, 255);		// Default opaque white edges

	//-----------------------------------------------------------------------------------------------
	// State
	//-----------------------------------------------------------------------------------------------
	bool m_isHighlighted = false;			// Whether this shape is currently selected/highlighted
};