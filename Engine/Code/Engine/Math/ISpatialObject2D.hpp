#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"

//-----------------------------------------------------------------------------------------------
// ISpatialObject2D - Interface for objects that can be stored in spatial structures
//
// Any object that wants to be stored in BVH/QuadTree must implement this interface.
// This allows the spatial structures to be completely generic and reusable.
//-----------------------------------------------------------------------------------------------
class ISpatialObject2D
{
public:
	virtual ~ISpatialObject2D() {}
	
	// Get the bounding box for this object (used for spatial partitioning)
	virtual AABB2 GetBounds() const = 0;
	
	// Get the center position (used for some partitioning strategies)
	virtual Vec2 GetCenter() const = 0;
	
	// Optional: Get bounding radius (for disc-based culling)
	virtual float GetBoundingRadius() const { return 0.f; }
};
