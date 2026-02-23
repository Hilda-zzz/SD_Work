#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/ConvexPoly2.hpp"
#include "Engine/Math/ConvexHull2.hpp"
#include "Engine/Math/ISpatialObject2D.hpp"

class RandomNumberGenerator;



class ConvexObject2 : public ISpatialObject2D
{
public:
	ConvexObject2();
	~ConvexObject2();

	virtual AABB2 GetBounds() const override;
	virtual Vec2 GetCenter() const override { return m_generateCenter; }
	virtual float GetBoundingRadius() const override { return m_generateRadius; }

	// Generate a random convex object
	void GenerateRandom(int numVertices, Vec2 const& center, float minRadius, float maxRadius, RandomNumberGenerator& rng);

	// Create specific shapes
	void MakeRegularPolygon(int numSides, Vec2 const& center, float radius);
	void MakeBox(Vec2 const& center, float width, float height);

	//-----------------------------------------------------------------------------------------------
	// Spatial Queries (delegates to hull/poly)
	//-----------------------------------------------------------------------------------------------
	bool ContainsPoint(Vec2 const& point) const;
	//RaycastResult2D Raycast(Vec2 const& rayStart, Vec2 const& rayEnd, bool generateDebugData = false) const;
	RaycastResult2D Raycast(Vec2 const& rayStart, Vec2 const& fwdNormal, float maxDist) const;
	// Bounding volume queries
	//float GetBoundingRadius() const;
	//Vec2 GetCenter() const;
	bool DoesRayIntersectBoundingDisc(Vec2 const& rayStart, Vec2 const& rayEnd) const;

	Vec2 GetGenerateCenter() const { return m_generateCenter; }
	float GetGenerateRadius() const { return m_generateRadius; }

	//-----------------------------------------------------------------------------------------------
	// Transformations
	//-----------------------------------------------------------------------------------------------
	// Transform the object around a specified pivot point
	void RotateAroundPoint(Vec2 const& pivot, float degrees);
	void ScaleAroundPoint(Vec2 const& pivot, float uniformScale);
	void Translate(Vec2 const& offset);

	// Combined transform for efficiency
	void TransformAroundPoint(Vec2 const& pivot, float rotationDegrees, float uniformScale);

	//-----------------------------------------------------------------------------------------------
	// Accessors / Mutators
	//-----------------------------------------------------------------------------------------------
	ConvexPoly2 const& GetPoly() const { return m_poly; }
	ConvexHull2 const& GetHull() const { return m_hull; }

	//-----------------------------------------------------------------------------------------------
	// Rendering (these will call DebugDraw functions)
	//-----------------------------------------------------------------------------------------------
	enum class RenderState
	{
		NORMAL,
		HIGHLIGHTED,
		DRAGGING
	};

	void SetRenderState(RenderState state) { m_renderState = state; }
	RenderState GetRenderState() const { return m_renderState; }

	void RenderFill(int drawMode) const;						
	void RenderEdges(int drawMode) const;			

	void RenderBoundingDisc() const;					
	void RenderEdgeNorms() const;

	// Debug rendering for single-object raycast visualization
	void RenderRaycastDebug(RaycastResult2D const& result) const;

private:
	//-----------------------------------------------------------------------------------------------
	// Internal helper methods
	//-----------------------------------------------------------------------------------------------
	void RebuildHullFromPoly();				// Rebuild hull after polygon changes
	void UpdateCachedBounds();				// Update cached center and radius

private:
	ConvexPoly2 m_poly;						// Polygon geometry (vertices)
	ConvexHull2 m_hull;						// Convex hull (planes for collision)

	//Vec2 m_cachedCenter = Vec2::ZERO;
	//float m_cachedBoundingRadius = 0.f;

	Vec2 m_generateCenter = Vec2::ZERO;
	float m_generateRadius = 0.f;
	AABB2 m_cachedAABB;

	RenderState m_renderState = RenderState::NORMAL;
	// Color constants
	static const Rgba8 COLOR_FILL_NORMAL;      
	static const Rgba8 COLOR_FILL_HIGHLIGHT; 
	static const Rgba8 COLOR_FILL_NORMAL_TRANS;
	static const Rgba8 COLOR_FILL_HIGHLIGHT_TRANS;
	static const Rgba8 COLOR_FILL_DRAGGING;     
		   const
	static const Rgba8 COLOR_EDGE_NORMAL;     
	static const Rgba8 COLOR_EDGE_HIGHLIGHT;     

	static const float EDGE_THICKNESS_NORMAL;
	static const float EDGE_THICKNESS_HIGHLIGHT;
};