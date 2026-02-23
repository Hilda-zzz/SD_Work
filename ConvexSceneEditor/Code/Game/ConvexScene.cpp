#include "Game/ConvexScene.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "GameCommon.hpp"
#include <Engine/Core/Time.hpp>
#include "Engine/Math/QuadTree2D.hpp"

//-----------------------------------------------------------------------------------------------
ConvexScene::ConvexScene()
{
}

//-----------------------------------------------------------------------------------------------
ConvexScene::~ConvexScene()
{
	ClearAllObjects();

	if (m_bvhTree)
	{
		delete m_bvhTree;
		m_bvhTree = nullptr;
	}

	if (m_quadTree)
	{
		delete m_quadTree;
		m_quadTree = nullptr;
	}
}

//-----------------------------------------------------------------------------------------------
void ConvexScene::AddObject(ConvexObject2* object)
{
	m_objects.push_back(object);
}

//-----------------------------------------------------------------------------------------------
void ConvexScene::RemoveObject(ConvexObject2* object)
{
	for (int i = 0; i < (int)m_objects.size(); ++i)
	{
		if (m_objects[i] == object)
		{
			m_objects.erase(m_objects.begin() + i);
			return;
		}
	}
}

//-----------------------------------------------------------------------------------------------
void ConvexScene::ClearAllObjects()
{
	m_topObject = nullptr;
	for (int i = 0; i < (int)m_objects.size(); ++i)
	{
		delete m_objects[i];
		m_objects[i] = nullptr;
	}
	m_objects.clear();
}

//-----------------------------------------------------------------------------------------------
void ConvexScene::RegenerateScene(int numObjects, RandomNumberGenerator& rng)
{
	ClearAllObjects();

	for (int i = 0; i < numObjects; ++i)
	{
		ConvexObject2* obj = new ConvexObject2();

		// Random number of vertices
		int numVertices = rng.RollRandomFloatInRange(m_minVertices, m_maxVertices);

		// Random center position within scene bounds
		Vec2 center(
			rng.RollRandomFloatInRange(m_sceneMin.x, m_sceneMax.x),
			rng.RollRandomFloatInRange(m_sceneMin.y, m_sceneMax.y)
		);

		// Random radius
		float minRadius = m_minObjectRadius;
		float maxRadius = m_maxObjectRadius;

		// Generate the object
		obj->GenerateRandom(numVertices, center, minRadius, maxRadius, rng);

		m_objects.push_back(obj);
	}
}

//-----------------------------------------------------------------------------------------------
ConvexObject2* ConvexScene::GetObjectAtPoint(Vec2 const& point) const
{
	// Return the last object that contains the point (topmost in render order)
	ConvexObject2* result = nullptr;

	for (int i = 0; i < (int)m_objects.size(); ++i)
	{
		if (m_objects[i]->ContainsPoint(point))
		{
			result = m_objects[i];  // Keep searching to find the topmost one
		}
	}

	return result;
}

//-----------------------------------------------------------------------------------------------
SceneRaycastResult ConvexScene::RaycastScene(std::vector<Ray> const& rays, RaycastModeConfig const& testConfig)
{
	SceneRaycastResult result;

	std::vector<ISpatialObject2D*> spatialCandidates;
	spatialCandidates.reserve(20);

	for (int i = 0; i < rays.size(); i++)
	{
		spatialCandidates.clear();
		switch (testConfig.m_mode)
		{
		case OptimizationMode::NONE:
			for (int j = 0; j < (int)m_objects.size(); ++j)
			{
				spatialCandidates.push_back(m_objects[j]);
			}
			break;
		case OptimizationMode::BVH:
			if (m_bvhTree)
			{
				m_bvhTree->QueryRay(rays[i].m_rayStartPos, rays[i].m_rayFwdNormal, rays[i].m_rayMaxLength, spatialCandidates);
			}
			break;
		case OptimizationMode::QUAD_TREE:
			if (m_quadTree)
			{
				m_quadTree->QueryRay(rays[i].m_rayStartPos, rays[i].m_rayFwdNormal,
					rays[i].m_rayMaxLength, spatialCandidates);
			}
			break;
		default:
			break;
		}

		for (int j = 0; j < (int)spatialCandidates.size(); ++j)
		{
			ConvexObject2* obj = static_cast<ConvexObject2*>(spatialCandidates[j]);
			// Per-Object Optimization Check
			if (testConfig.m_useDiscCheck)
			{
				RaycastResult2D discResult = RaycastVsDisc2D(rays[i].m_rayStartPos, rays[i].m_rayFwdNormal, rays[i].m_rayMaxLength,
					obj->GetGenerateCenter(), obj->GetGenerateRadius());
				if (!discResult.m_didImpact)
					continue;
			}

			obj->Raycast(rays[i].m_rayStartPos, rays[i].m_rayFwdNormal, rays[i].m_rayMaxLength);
			result.m_numObjectsTested++;
			if (testConfig.m_useDiscCheck)
			{
				result.m_numDiscTests++;
			}
		}
		
	}
	
	return result;
}

SceneRaycastResult ConvexScene::RaycastWithBVH(Vec2 const& rayStart, Vec2 const& rayDir, float maxDist) const
{
	SceneRaycastResult result;

	if (!m_bvhTree)
		return result;

	// 查询返回ISpatialObject2D*
	std::vector<ISpatialObject2D*> spatialCandidates;
	m_bvhTree->QueryRay(rayStart, rayDir, maxDist, spatialCandidates);

	// 转换回ConvexObject2*进行精确测试
	float closestDist = maxDist;

	for (int i = 0; i < (int)spatialCandidates.size(); ++i)
	{
		// 向下转型回具体类型
		ConvexObject2* obj = static_cast<ConvexObject2*>(spatialCandidates[i]);

		RaycastResult2D objResult = obj->Raycast(rayStart, rayDir, maxDist);

		if (objResult.m_didImpact && objResult.m_impactDist < closestDist)
		{
			closestDist = objResult.m_impactDist;
			result.m_didImpact = true;
			result.m_impactPosition = objResult.m_impactPos;
			result.m_impactNormal = objResult.m_impactNormal;
			result.m_impactDistance = objResult.m_impactDist;
			result.m_impactFraction = objResult.m_impactDist / maxDist;
			result.m_impactedObject = obj;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------------------------
//float ConvexScene::PerformBatchRaycast(int numRays, OptimizationMode mode, RandomNumberGenerator& rng)
//{
//	Vec2 sceneMin(100.f, 100.f);
//	Vec2 sceneMax(1500.f, 700.f);
//
//	struct Ray
//	{
//		Vec2 start;
//		Vec2 end;
//	};
//
//	std::vector<Ray> rays;
//	rays.reserve(numRays);
//
//	for (int i = 0; i < numRays; ++i)
//	{
//		Ray ray;
//		ray.start = Vec2(
//			rng.RollRandomFloatInRange(sceneMin.x, sceneMax.x),
//			rng.RollRandomFloatInRange(sceneMin.y, sceneMax.y)
//		);
//		ray.end = Vec2(
//			rng.RollRandomFloatInRange(sceneMin.x, sceneMax.x),
//			rng.RollRandomFloatInRange(sceneMin.y, sceneMax.y)
//		);
//		rays.push_back(ray);
//	}
//
//	// ===== 开始计时 =====
//	double startTime = GetCurrentTimeSeconds();
//
//	// 执行所有射线测试
//	std::vector<ConvexObject2*> const& objects =GetAllObjects();
//	for (int i = 0; i < numRays; ++i)
//	{
//		for (int j = 0; j < (int)objects.size(); ++j)
//		{
//			objects[j]->Raycast(rays[i].start, rays[i].end, false);
//		}
//	}
//
//	// ===== 结束计时 =====
//	double endTime = GetCurrentTimeSeconds();
//	//m_lastRaycastTimeMs = (float)((endTime - startTime) * 1000.0);
//}

//-----------------------------------------------------------------------------------------------
//void ConvexScene::RebuildSpatialHash()
//{
//	// TODO: Implement spatial hash
//}
//
////-----------------------------------------------------------------------------------------------
//void ConvexScene::RebuildAABBTree()
//{
//	// TODO: Implement AABB tree
//}
//
////-----------------------------------------------------------------------------------------------
//void ConvexScene::ClearSpatialStructures()
//{
//	// TODO: Implement
//}

void ConvexScene::RebuildBVHTree(int maxObjectsPerLeaf, int maxTreeDepth)
{
	if (m_bvhTree)
		delete m_bvhTree;

	std::vector<ISpatialObject2D*> spatialObjects;
	spatialObjects.reserve(m_objects.size());

	for (int i = 0; i < (int)m_objects.size(); ++i)
	{
		spatialObjects.push_back(m_objects[i]);  
	}

	m_bvhTree = new BVHTree2D();
	m_bvhTree->Build(spatialObjects, m_sceneMin, m_sceneMax, maxObjectsPerLeaf, maxTreeDepth);
}

void ConvexScene::RebuildQuadTree(int maxObjectsPerLeaf, int maxTreeDepth)
{
	if (m_quadTree)
		delete m_quadTree;

	std::vector<ISpatialObject2D*> spatialObjects;
	spatialObjects.reserve(m_objects.size());

	for (int i = 0; i < (int)m_objects.size(); ++i)
	{
		spatialObjects.push_back(m_objects[i]);
	}

	m_quadTree = new QuadTree2D();
	m_quadTree->Build(spatialObjects,maxObjectsPerLeaf, maxTreeDepth);
}

//-----------------------------------------------------------------------------------------------
char const* ConvexScene::GetOptimizationModeName() const
{
	switch (m_currentOptimizationMode)
	{
	case OptimizationMode::NONE:
		return "None (Brute Force)";
	case OptimizationMode::BVH:
		return "BVH";
	case OptimizationMode::QUAD_TREE:
		return "Quad Tree";
	default:
		return "Unknown";
	}
}

//-----------------------------------------------------------------------------------------------
void ConvexScene::RenderScene(int drawMode) const
{
	// 找到高亮或拖拽的对象（需要最后渲染）
	ConvexObject2* topObject = m_topObject;
	//for (int i = 0; i < (int)m_objects.size(); ++i)
	//{
	//	if (m_objects[i]->GetRenderState() == ConvexObject2::RenderState::HIGHLIGHTED ||
	//		m_objects[i]->GetRenderState() == ConvexObject2::RenderState::DRAGGING)
	//	{
	//		topObject = m_objects[i];
	//		break;
	//	}
	//}

	if (drawMode == 0)
	{
		for (int i = 0; i < (int)m_objects.size(); ++i)
		{
			if (m_objects[i] != topObject)
			{
				m_objects[i]->RenderEdges(0);
			}
		}

		for (int i = 0; i < (int)m_objects.size(); ++i)
		{
			if (m_objects[i] != topObject)
			{
				m_objects[i]->RenderFill(0);
			}
		}

		if (topObject)
		{
			topObject->RenderEdges(0);
			topObject->RenderFill(0);
		}
	}
	else if (drawMode == 1)
	{
		for (int i = 0; i < (int)m_objects.size(); ++i)
		{
			if (m_objects[i] != topObject)
			{
				m_objects[i]->RenderEdges(1);
				m_objects[i]->RenderFill(1);
			}
		}
		if (topObject)
		{
			topObject->RenderEdges(1);
			topObject->RenderFill(1);
		}
	}
	//RenderTestPlanes();
}

//-----------------------------------------------------------------------------------------------
void ConvexScene::RenderBoundingDiscs() const
{
	for (int i = 0; i < (int)m_objects.size(); ++i)
	{
		m_objects[i]->RenderBoundingDisc();
	}
}

//-----------------------------------------------------------------------------------------------
void ConvexScene::RenderSpatialStructures() const
{
	// TODO: Implement
}

void ConvexScene::GenerateRandomPlanes(int numPlanes, RandomNumberGenerator& rng)
{
	m_testPlanes.clear();

	for (int i = 0; i < numPlanes; ++i)
	{
		// 随机生成两个点
		Vec2 pointA(rng.RollRandomFloatInRange(m_sceneMin.x, m_sceneMax.x),
			rng.RollRandomFloatInRange(m_sceneMin.y, m_sceneMax.y));
		Vec2 pointB(rng.RollRandomFloatInRange(m_sceneMin.x, m_sceneMax.x),
			rng.RollRandomFloatInRange(m_sceneMin.y, m_sceneMax.y));

		Plane2 plane = Plane2::FromPoints(pointA, pointB);
		m_testPlanes.push_back(plane);
	}
}

void ConvexScene::RenderTestPlanes() const
{
	for (int i = 0; i < (int)m_testPlanes.size(); ++i)
	{
		Plane2 const& plane = m_testPlanes[i];

		// 找到平面上的一个点
		Vec2 pointOnPlane = plane.m_normal * plane.m_distance;

		// 沿着平面方向绘制线段（垂直于法线）
		Vec2 planeDir = Vec2(-plane.m_normal.y, plane.m_normal.x); // 旋转90度
		Vec2 lineStart = pointOnPlane - planeDir * 200.f;
		Vec2 lineEnd = pointOnPlane + planeDir * 200.f;

		// 绘制平面线（白色）
		DebugDrawLine(lineStart, lineEnd, 2.f, Rgba8::WHITE);
		DebugDrawCircle(3.f, lineStart, Rgba8::GREEN);

		// 绘制法线箭头（黄色）
		Vec2 normalEnd = pointOnPlane + plane.m_normal * 30.f;
		DebugDrawLine(pointOnPlane, normalEnd, 2.f, Rgba8::YELLOW);
	}
}

//-----------------------------------------------------------------------------------------------
SceneRaycastResult ConvexScene::RaycastBruteForce(Vec2 const& rayStart, Vec2 const& rayEnd) const
{
	// TODO: Implement
	SceneRaycastResult result;
	return result;
}

//-----------------------------------------------------------------------------------------------
SceneRaycastResult ConvexScene::RaycastWithBoundingDiscs(Vec2 const& rayStart, Vec2 const& rayEnd) const
{
	// TODO: Implement
	SceneRaycastResult result;
	return result;
}

//-----------------------------------------------------------------------------------------------
SceneRaycastResult ConvexScene::RaycastWithSpatialHash(Vec2 const& rayStart, Vec2 const& rayEnd) const
{
	// TODO: Implement
	SceneRaycastResult result;
	return result;
}

//-----------------------------------------------------------------------------------------------
SceneRaycastResult ConvexScene::RaycastWithAABBTree(Vec2 const& rayStart, Vec2 const& rayEnd) const
{
	// TODO: Implement
	SceneRaycastResult result;
	return result;
}

//-----------------------------------------------------------------------------------------------
SceneRaycastResult ConvexScene::RaycastWithBothOptimizations(Vec2 const& rayStart, Vec2 const& rayEnd) const
{
	// TODO: Implement
	SceneRaycastResult result;
	return result;
}
