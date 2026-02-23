#include "BVHTree2D.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
//#include "Game/GameCommon.hpp"
#include <algorithm>
#include <Engine/Core/EngineCommon.hpp>
#include <Engine/Core/Vertex_PCU.hpp>
#include <Engine/Core/VertexUtils.hpp>
#include "Engine/Renderer/Renderer.hpp"
extern Renderer* g_theRenderer;
//-----------------------------------------------------------------------------------------------
BVHTree2D::BVHTree2D()
{
}

//-----------------------------------------------------------------------------------------------
BVHTree2D::~BVHTree2D()
{
	Clear();
}

//-----------------------------------------------------------------------------------------------
void BVHTree2D::Build(std::vector<ISpatialObject2D*> const& objects, Vec2 sceneMin, Vec2 sceneMax)
{
	UNUSED(sceneMin);
	UNUSED(sceneMax);
	
	// Clear any existing tree
	Clear();
	
	if (objects.empty())
		return;
	
	m_totalObjects = (int)objects.size();
	
	// Build the tree recursively
	m_root = BuildRecursive(objects, 0);
}

void BVHTree2D::Build(std::vector<ISpatialObject2D*> const& objects, Vec2 sceneMin, Vec2 sceneMax, int maxObjectsPerLeaf, int maxTreeDepth)
{
	m_maxObjectsPerLeaf = maxObjectsPerLeaf;
	m_maxTreeDepth = maxTreeDepth;

	Build(objects, sceneMin, sceneMax);
}

//-----------------------------------------------------------------------------------------------
void BVHTree2D::Clear()
{
	if (m_root)
	{
		ClearRecursive(m_root);
		m_root = nullptr;
	}
	m_maxDepth = 0;
	m_nodeCount = 0;
	m_totalObjects = 0;
}

//-----------------------------------------------------------------------------------------------
void BVHTree2D::QueryRay(Vec2 const& rayStart, Vec2 const& rayDir, float maxDist,
						 std::vector<ISpatialObject2D*>& outCandidates) const
{
	outCandidates.clear();
	
	if (!m_root)
		return;
	
	QueryRayRecursive(m_root, rayStart, rayDir, maxDist, outCandidates);
}

//-----------------------------------------------------------------------------------------------
void BVHTree2D::QueryAABB(AABB2 const& bounds, std::vector<ISpatialObject2D*>& outCandidates) const
{
	outCandidates.clear();
	
	if (!m_root)
		return;
	
	QueryAABBRecursive(m_root, bounds, outCandidates);
}

//-----------------------------------------------------------------------------------------------
void BVHTree2D::QueryPoint(Vec2 const& point, std::vector<ISpatialObject2D*>& outCandidates) const
{
	outCandidates.clear();
	
	if (!m_root)
		return;
	
	QueryPointRecursive(m_root, point, outCandidates);
}

//-----------------------------------------------------------------------------------------------
AABB2 BVHTree2D::GetRootBounds() const
{
	if (m_root)
		return m_root->m_bounds;
	return AABB2();
}

//-----------------------------------------------------------------------------------------------
void BVHTree2D::DebugRender(int maxDepthToShow) const
{
	if (!m_root)
		return;
	
	int maxDepth = (maxDepthToShow < 0) ? m_maxDepth : maxDepthToShow;
	DebugRenderRecursive(m_root, 0, maxDepth);
}

//-----------------------------------------------------------------------------------------------
// PRIVATE METHODS
//-----------------------------------------------------------------------------------------------

BVHNode* BVHTree2D::BuildRecursive(std::vector<ISpatialObject2D*> const& objects, int depth)
{
	if (objects.empty())
		return nullptr;
	
	// Create node
	BVHNode* node = new BVHNode();
	node->m_bounds = ComputeBounds(objects);
	m_nodeCount++;
	
	if (depth > m_maxDepth)
		m_maxDepth = depth;
	
	// Check if we should make this a leaf node
	bool shouldSplit = (int)objects.size() > m_maxObjectsPerLeaf && depth < m_maxTreeDepth;
	
	if (!shouldSplit)
	{
		// Make this a leaf node
		node->m_objects = objects;
		return node;
	}
	
	// TODO: Split into left and right children
	// 1. Find best split axis (X or Y)
	// 2. Sort objects along that axis by their centers
	// 3. Split at midpoint
	// 4. Recursively build left and right subtrees
	
	//// For now, just make it a leaf (placeholder)
	//node->m_objects = objects;
	//return node;
	
	
	// Example split implementation (uncomment when ready to implement):
	// 0 = X axis, 1 = Y axis
	int splitAxis = FindBestSplitAxis(objects, node->m_bounds);
	
	// Sort objects by center along split axis
	std::vector<ISpatialObject2D*> sortedObjects = objects;
	std::sort(sortedObjects.begin(), sortedObjects.end(), 
		[splitAxis](ISpatialObject2D* a, ISpatialObject2D* b) {
			Vec2 centerA = a->GetCenter();
			Vec2 centerB = b->GetCenter();
			return (splitAxis == 0) ? (centerA.x < centerB.x) : (centerA.y < centerB.y);
		});
	
	// Split at midpoint
	int midIndex = (int)sortedObjects.size() / 2;
	std::vector<ISpatialObject2D*> leftObjects(sortedObjects.begin(), sortedObjects.begin() + midIndex);
	std::vector<ISpatialObject2D*> rightObjects(sortedObjects.begin() + midIndex, sortedObjects.end());
	
	// Recursively build children
	node->m_left = BuildRecursive(leftObjects, depth + 1);
	node->m_right = BuildRecursive(rightObjects, depth + 1);
	
	return node;
	
}

//-----------------------------------------------------------------------------------------------
AABB2 BVHTree2D::ComputeBounds(std::vector<ISpatialObject2D*> const& objects) const
{
	if (objects.empty())
		return AABB2();
	
	AABB2 result = objects[0]->GetBounds();
	
	for (int i = 1; i < (int)objects.size(); ++i)
	{
		AABB2 objBounds = objects[i]->GetBounds();
		
		result.m_mins.x = (objBounds.m_mins.x < result.m_mins.x) ? objBounds.m_mins.x : result.m_mins.x;
		result.m_mins.y = (objBounds.m_mins.y < result.m_mins.y) ? objBounds.m_mins.y : result.m_mins.y;
		result.m_maxs.x = (objBounds.m_maxs.x > result.m_maxs.x) ? objBounds.m_maxs.x : result.m_maxs.x;
		result.m_maxs.y = (objBounds.m_maxs.y > result.m_maxs.y) ? objBounds.m_maxs.y : result.m_maxs.y;
	}
	
	return result;
}

//-----------------------------------------------------------------------------------------------
int BVHTree2D::FindBestSplitAxis(std::vector<ISpatialObject2D*> const& objects, AABB2 const& bounds) const
{
	UNUSED(objects);
	
	// Simple strategy: split along the longer axis
	Vec2 extent = bounds.GetDimensions();
	return (extent.x > extent.y) ? 0 : 1;  // 0 = X axis, 1 = Y axis
}

//-----------------------------------------------------------------------------------------------
void BVHTree2D::QueryRayRecursive(BVHNode* node, Vec2 const& rayStart, Vec2 const& rayDir,
								  float maxDist, std::vector<ISpatialObject2D*>& outCandidates) const
{
	if (!node)
		return;
	
	Vec2 rayEnd = rayStart + rayDir * maxDist;

	AABB2 rayBounds;
	rayBounds.m_mins.x = (rayStart.x < rayEnd.x) ? rayStart.x : rayEnd.x;
	rayBounds.m_mins.y = (rayStart.y < rayEnd.y) ? rayStart.y : rayEnd.y;
	rayBounds.m_maxs.x = (rayStart.x > rayEnd.x) ? rayStart.x : rayEnd.x;
	rayBounds.m_maxs.y = (rayStart.y > rayEnd.y) ? rayStart.y : rayEnd.y;

	if (!DoAABB2Overlap(node->m_bounds, rayBounds))
		return;
	    
	if (node->IsLeaf())
	{
		// Add all objects from this leaf
		for (int i = 0; i < (int)node->m_objects.size(); ++i)
		{
			outCandidates.push_back(node->m_objects[i]);
		}
	}
	else
	{
		// Recursively query children
		QueryRayRecursive(node->m_left, rayStart, rayDir, maxDist, outCandidates);
		QueryRayRecursive(node->m_right, rayStart, rayDir, maxDist, outCandidates);
	}
}

//-----------------------------------------------------------------------------------------------
void BVHTree2D::QueryAABBRecursive(BVHNode* node, AABB2 const& bounds,
								   std::vector<ISpatialObject2D*>& outCandidates) const
{
	if (!node)
		return;
	
	// Test if query bounds overlap node bounds
	if (!DoAABB2Overlap(node->m_bounds, bounds)) 
		return;
	
	if (node->IsLeaf())
	{
		// Add all objects from this leaf
		for (int i = 0; i < (int)node->m_objects.size(); ++i)
		{
			outCandidates.push_back(node->m_objects[i]);
		}
	}
	else
	{
		// Recursively query children
		QueryAABBRecursive(node->m_left, bounds, outCandidates);
		QueryAABBRecursive(node->m_right, bounds, outCandidates);
	}
}

//-----------------------------------------------------------------------------------------------
void BVHTree2D::QueryPointRecursive(BVHNode* node, Vec2 const& point,
									std::vector<ISpatialObject2D*>& outCandidates) const
{
	if (!node)
		return;
	
	// Test if point is inside node bounds
	if (!node->m_bounds.IsPointInside(point))
		return;
	
	if (node->IsLeaf())
	{
		// Add all objects from this leaf
		for (int i = 0; i < (int)node->m_objects.size(); ++i)
		{
			outCandidates.push_back(node->m_objects[i]);
		}
	}
	else
	{
		// Recursively query children
		QueryPointRecursive(node->m_left, point, outCandidates);
		QueryPointRecursive(node->m_right, point, outCandidates);
	}
}

//-----------------------------------------------------------------------------------------------
void BVHTree2D::ClearRecursive(BVHNode* node)
{
	if (!node)
		return;
	
	ClearRecursive(node->m_left);
	ClearRecursive(node->m_right);
	
	delete node;
}

//-----------------------------------------------------------------------------------------------
void BVHTree2D::DebugRenderRecursive(BVHNode* node, int currentDepth, int maxDepth) const
{
	if (!node || currentDepth > maxDepth)
		return;
	
	// Draw bounds with color based on depth and leaf status
	Rgba8 color;
	if (node->IsLeaf())
	{
		color = Rgba8(0, 255, 0, 255);  // Green for leaves
	}
	else
	{
		color = Rgba8(255 - currentDepth * 20, 255 - currentDepth * 20, 255, 255);
	}
	
	Vec2 min = node->m_bounds.m_mins;
	Vec2 max = node->m_bounds.m_maxs;
	//g_theRenderer->SetModelConstants();
	std::vector<Vertex_PCU> boxVerts;
	AddVertsForAABBWire2D(boxVerts, node->m_bounds, color, 3.f, false);
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(boxVerts);
	
	// Recursively render children
	DebugRenderRecursive(node->m_left, currentDepth + 1, maxDepth);
	DebugRenderRecursive(node->m_right, currentDepth + 1, maxDepth);
}
