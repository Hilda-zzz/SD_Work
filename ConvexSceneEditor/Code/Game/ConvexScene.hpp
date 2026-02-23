#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Game/ConvexObject2.hpp"
#include <vector>
#include "Engine/Math/BVHTree2D.hpp"

class RandomNumberGenerator;
class QuadTree2D;
//class SpatialHash2D;
//class AABBTree2D;

//-----------------------------------------------------------------------------------------------
// SceneRaycastResult - Extended raycast result that includes which object was hit
//-----------------------------------------------------------------------------------------------
struct SceneRaycastResult
{
	bool m_didImpact = false;
	Vec2 m_impactPosition = Vec2::ZERO;
	Vec2 m_impactNormal = Vec2::ZERO;
	float m_impactDistance = 0.f;
	float m_impactFraction = 0.f;
	ConvexObject2* m_impactedObject = nullptr;

	// Performance metrics (for debugging/profiling)
	int m_numObjectsTested = 0;				// How many objects were tested
	int m_numDiscTests = 0;					// How many bounding disc tests
	int m_numDiscRejections = 0;			// How many rejected by disc test
};

enum class OptimizationMode
{
	NONE = 0,						// No optimization, test all objects
	BVH=1,
	QUAD_TREE=2,
	NUM_MODES
};

struct RaycastModeConfig
{
	OptimizationMode m_mode;
	bool m_useDiscCheck;
};

class ConvexScene
{
public:
	ConvexScene();
	~ConvexScene();

	void AddObject(ConvexObject2* object);
	void RemoveObject(ConvexObject2* object);
	void ClearAllObjects();
	void RegenerateScene(int numObjects, RandomNumberGenerator& rng);

	int GetObjectCount() const { return (int)m_objects.size(); }

	//-----------------------------------------------------------------------------------------------
	// Spatial Queries
	//-----------------------------------------------------------------------------------------------
	// Find the topmost object containing this point (nullptr if none)
	// If multiple objects contain the point, returns the last one in the list
	ConvexObject2* GetObjectAtPoint(Vec2 const& point) const;

	// Raycast against all objects in the scene
	// Returns the closest hit along the ray
	SceneRaycastResult RaycastScene(std::vector<Ray> const& rays,RaycastModeConfig const& testConfig);

	SceneRaycastResult RaycastWithBVH(Vec2 const& rayStart, Vec2 const& rayDir,
		float maxDist) const;
	//-----------------------------------------------------------------------------------------------
	// Spatial Acceleration Structures
	//-----------------------------------------------------------------------------------------------
	void RebuildBVHTree(int maxObjectsPerLeaf = 3, int maxTreeDepth = 16);
	void RebuildQuadTree(int maxObjectsPerLeaf = 8, int maxTreeDepth = 8);

	//-----------------------------------------------------------------------------------------------
	// Accessors
	//-----------------------------------------------------------------------------------------------
	std::vector<ConvexObject2*> const& GetAllObjects() const { return m_objects; }
	OptimizationMode GetOptimizationMode() const { return m_currentOptimizationMode; }
	void SetOptimizationMode(OptimizationMode mode) { m_currentOptimizationMode = mode; }

	// Get name of current optimization mode for display
	char const* GetOptimizationModeName() const;

	void SetTopObject(ConvexObject2* obj) { m_topObject = obj; }

	//-----------------------------------------------------------------------------------------------
	// Rendering
	//-----------------------------------------------------------------------------------------------
	// Render all objects in the scene
	// drawMode: 0 = composite (all edges then all fills), 1 = individual (each object fully rendered)
	void RenderScene(int drawMode = 0) const;
	void RenderBoundingDiscs() const;
	void RenderSpatialStructures() const;

	//---------------Test Plane2 ----------------------------
	void GenerateRandomPlanes(int numPlanes, RandomNumberGenerator& rng);
	void RenderTestPlanes() const;
	std::vector<Plane2> const& GetTestPlanes() const { return m_testPlanes; }

private:
	//-----------------------------------------------------------------------------------------------
	// Internal helper methods
	//-----------------------------------------------------------------------------------------------
	// Different raycast implementations based on optimization mode
	SceneRaycastResult RaycastBruteForce(Vec2 const& rayStart, Vec2 const& rayEnd) const;
	SceneRaycastResult RaycastWithBoundingDiscs(Vec2 const& rayStart, Vec2 const& rayEnd) const;
	SceneRaycastResult RaycastWithSpatialHash(Vec2 const& rayStart, Vec2 const& rayEnd) const;
	SceneRaycastResult RaycastWithAABBTree(Vec2 const& rayStart, Vec2 const& rayEnd) const;
	SceneRaycastResult RaycastWithBothOptimizations(Vec2 const& rayStart, Vec2 const& rayEnd) const;

public:
	BVHTree2D* m_bvhTree = nullptr;
	QuadTree2D* m_quadTree = nullptr;
private:
	std::vector<ConvexObject2*> m_objects;		// Scene owns all objects (will delete in destructor)
	std::vector<Plane2> m_testPlanes;

	ConvexObject2* m_topObject = nullptr;	

	//-----------------------------------------------------------------------------------------------
	// Configuration
	//-----------------------------------------------------------------------------------------------
	OptimizationMode m_currentOptimizationMode = OptimizationMode::NONE;

	//-----------------------------------------------------------------------------------------------
	// Scene bounds (for random generation)
	//-----------------------------------------------------------------------------------------------
	Vec2 m_sceneMin = Vec2(100.f, 100.f);
	Vec2 m_sceneMax = Vec2(1500.f, 700.f);
	float m_minObjectRadius = 20.f;
	float m_maxObjectRadius = 130.f;
	int m_minVertices = 3;
	int m_maxVertices = 8;

	int m_totalHit = 0;
};