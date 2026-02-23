#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "ISpatialObject2D.hpp"
#include <vector>

struct BVHNode
{
	AABB2 m_bounds;							
	BVHNode* m_left = nullptr;				
	BVHNode* m_right = nullptr;		
	
	std::vector<ISpatialObject2D*> m_objects;	
	
	bool IsLeaf() const { return m_left == nullptr && m_right == nullptr; }
};

//-----------------------------------------------------------------------------------------------
// BVHTree2D - Generic hierarchical bounding volume tree
//
// Build strategy: Top-down recursive splitting along the axis with greatest extent
// Query strategy: Recursive traversal, testing against node bounds before children
//-----------------------------------------------------------------------------------------------
class BVHTree2D
{
public:
	BVHTree2D();
	~BVHTree2D();
	
	void Build(std::vector<ISpatialObject2D*> const& objects, Vec2 sceneMin, Vec2 sceneMax);
	void Build(std::vector<ISpatialObject2D*> const& objects, Vec2 sceneMin, Vec2 sceneMax,
		int maxObjectsPerLeaf, int maxTreeDepth);
	void Clear();
	
	//-----------------------------------------------------------------------------------------------
	// Queries
	//-----------------------------------------------------------------------------------------------
	// Find all objects that potentially intersect with the ray
	// Returns candidates that need precise testing
	void QueryRay(Vec2 const& rayStart, Vec2 const& rayDir, float maxDist,
				  std::vector<ISpatialObject2D*>& outCandidates) const;
	
	// Find all objects that potentially overlap with a bounding box
	void QueryAABB(AABB2 const& bounds, std::vector<ISpatialObject2D*>& outCandidates) const;
	
	// Find all objects that potentially contain a point
	void QueryPoint(Vec2 const& point, std::vector<ISpatialObject2D*>& outCandidates) const;
	
	//-----------------------------------------------------------------------------------------------
	// Statistics / Debug
	//-----------------------------------------------------------------------------------------------
	int GetDepth() const { return m_maxDepth; }
	int GetNodeCount() const { return m_nodeCount; }
	int GetObjectCount() const { return m_totalObjects; }
	AABB2 GetRootBounds() const;
	
	// Debug rendering - draws bounding boxes
	// maxDepthToShow: -1 means show all levels, otherwise limit to this depth
	void DebugRender(int maxDepthToShow = -1) const;
	
	//-----------------------------------------------------------------------------------------------
	// Configuration
	//-----------------------------------------------------------------------------------------------
	void SetMaxObjectsPerLeaf(int max) { m_maxObjectsPerLeaf = max; }
	void SetMaxTreeDepth(int max) { m_maxTreeDepth = max; }
	
private:
	//-----------------------------------------------------------------------------------------------
	// Internal construction helpers
	//-----------------------------------------------------------------------------------------------
	BVHNode* BuildRecursive(std::vector<ISpatialObject2D*> const& objects, int depth);
	AABB2 ComputeBounds(std::vector<ISpatialObject2D*> const& objects) const;
	int FindBestSplitAxis(std::vector<ISpatialObject2D*> const& objects, AABB2 const& bounds) const;
	
	//-----------------------------------------------------------------------------------------------
	// Internal query helpers
	//-----------------------------------------------------------------------------------------------
	void QueryRayRecursive(BVHNode* node, Vec2 const& rayStart, Vec2 const& rayDir, 
						   float maxDist, std::vector<ISpatialObject2D*>& outCandidates) const;
	void QueryAABBRecursive(BVHNode* node, AABB2 const& bounds, 
						    std::vector<ISpatialObject2D*>& outCandidates) const;
	void QueryPointRecursive(BVHNode* node, Vec2 const& point, 
						     std::vector<ISpatialObject2D*>& outCandidates) const;
	
	//-----------------------------------------------------------------------------------------------
	// Internal cleanup helpers
	//-----------------------------------------------------------------------------------------------
	void ClearRecursive(BVHNode* node);
	
	//-----------------------------------------------------------------------------------------------
	// Internal debug helpers
	//-----------------------------------------------------------------------------------------------
	void DebugRenderRecursive(BVHNode* node, int currentDepth, int maxDepth) const;
	
private:
	BVHNode* m_root = nullptr;
	
	// Statistics
	int m_maxDepth = 0;
	int m_nodeCount = 0;
	int m_totalObjects = 0;
	
	// Configuration
	int m_maxObjectsPerLeaf = 3;		// Maximum objects in a leaf before splitting
	int m_maxTreeDepth = 16;			// Maximum tree depth to prevent infinite recursion
};
