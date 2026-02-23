#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "ISpatialObject2D.hpp"
#include <vector>
#include "Engine/Core/Rgba8.hpp"

//-----------------------------------------------------------------------------------------------
// QuadTreeNode - Node in the quad tree
//-----------------------------------------------------------------------------------------------
struct QuadTreeNode
{
	AABB2 m_bounds;	
	QuadTreeNode* m_children[4] = { nullptr, nullptr, nullptr, nullptr };  // NE, NW, SW, SE

	std::vector<ISpatialObject2D*> m_objects;	// Objects in this node (only for leaves)

	bool IsLeaf() const
	{
		return m_children[0] == nullptr && m_children[1] == nullptr &&
			m_children[2] == nullptr && m_children[3] == nullptr;
	}

	// Child indices
	enum ChildIndex
	{
		CHILD_NE = 0,  // (top-right)
		CHILD_NW = 1,  // (top-left)
		CHILD_SW = 2,  // (bottom-left)
		CHILD_SE = 3   // (bottom-right)
	};
};

//-----------------------------------------------------------------------------------------------
// QuadTree2D - Spatial partitioning tree that divides space into 4 quadrants
//
// Build strategy: Top-down recursive subdivision into 4 equal quadrants
// Query strategy: Recursive traversal, testing against node bounds before children
//-----------------------------------------------------------------------------------------------
class QuadTree2D
{
public:
	QuadTree2D();
	~QuadTree2D();

	void Build(std::vector<ISpatialObject2D*> const& objects);
	void Build(std::vector<ISpatialObject2D*> const& objects,
		int maxObjectsPerLeaf, int maxTreeDepth);
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
	void DebugRenderWithObjectColoring(int maxDepthToShow = -1) const;
	//-----------------------------------------------------------------------------------------------
	// Configuration
	//-----------------------------------------------------------------------------------------------
	void SetMaxObjectsPerLeaf(int max) { m_maxObjectsPerLeaf = max; }
	void SetMaxTreeDepth(int max) { m_maxTreeDepth = max; }

private:
	//-----------------------------------------------------------------------------------------------
	// Internal construction helpers
	//-----------------------------------------------------------------------------------------------
	QuadTreeNode* BuildRecursive(std::vector<ISpatialObject2D*> const& objects,
		AABB2 const& bounds, int depth);
	void SubdivideNode(QuadTreeNode* node, AABB2 childBounds[4]) const;
	int ClassifyObject(ISpatialObject2D* obj, AABB2 const& quadrant) const;

	//-----------------------------------------------------------------------------------------------
	// Internal query helpers
	//-----------------------------------------------------------------------------------------------
	void QueryRayRecursive(QuadTreeNode* node, Vec2 const& rayStart, Vec2 const& rayDir,
		float maxDist, std::vector<ISpatialObject2D*>& outCandidates) const;
	void QueryAABBRecursive(QuadTreeNode* node, AABB2 const& bounds,
		std::vector<ISpatialObject2D*>& outCandidates) const;
	void QueryPointRecursive(QuadTreeNode* node, Vec2 const& point,
		std::vector<ISpatialObject2D*>& outCandidates) const;

	//-----------------------------------------------------------------------------------------------
	// Internal cleanup helpers
	//-----------------------------------------------------------------------------------------------
	void ClearRecursive(QuadTreeNode* node);

	//-----------------------------------------------------------------------------------------------
	// Internal debug helpers
	//-----------------------------------------------------------------------------------------------
	void DebugRenderRecursive(QuadTreeNode* node, int currentDepth, int maxDepth) const;
	void DebugRenderRecursiveWithColoring(QuadTreeNode* node, int currentDepth, int maxDepth) const;
	Rgba8 GetColorForDepth(int depth) const;
	AABB2 ComputeSceneBounds(std::vector<ISpatialObject2D*> const& objects) const;
private:
	QuadTreeNode* m_root = nullptr;

	// Scene bounds
	Vec2 m_sceneMin = Vec2::ZERO;
	Vec2 m_sceneMax = Vec2::ZERO;

	// Statistics
	int m_maxDepth = 0;
	int m_nodeCount = 0;
	int m_totalObjects = 0;

	// Configuration
	int m_maxObjectsPerLeaf = 8;		// Maximum objects in a leaf before splitting
	int m_maxTreeDepth = 8;				// Maximum tree depth to prevent infinite recursion
};