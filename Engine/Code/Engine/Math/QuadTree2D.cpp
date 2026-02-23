#include "QuadTree2D.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <Engine/Core/VertexUtils.hpp>
#include "Engine/Renderer/Renderer.hpp"

extern Renderer* g_theRenderer;
//-----------------------------------------------------------------------------------------------
QuadTree2D::QuadTree2D()
{
}

//-----------------------------------------------------------------------------------------------
QuadTree2D::~QuadTree2D()
{
	Clear();
}

void QuadTree2D::Build(std::vector<ISpatialObject2D*> const& objects)
{
	Clear();

	if (objects.empty())
		return;

	// 自动计算边界
	AABB2 sceneBounds = ComputeSceneBounds(objects);
	m_sceneMin = sceneBounds.m_mins;
	m_sceneMax = sceneBounds.m_maxs;
	m_totalObjects = (int)objects.size();

	// 使用默认配置构建
	m_root = BuildRecursive(objects, sceneBounds, 0);
}

void QuadTree2D::Build(std::vector<ISpatialObject2D*> const& objects, int maxObjectsPerLeaf, int maxTreeDepth)
{
	m_maxObjectsPerLeaf = maxObjectsPerLeaf;
	m_maxTreeDepth = maxTreeDepth;

	// 调用自动边界版本
	Build(objects);
}

//-----------------------------------------------------------------------------------------------
void QuadTree2D::Build(std::vector<ISpatialObject2D*> const& objects, Vec2 sceneMin, Vec2 sceneMax)
{
	// Clear any existing tree
	Clear();

	if (objects.empty())
		return;

	m_sceneMin = sceneMin;
	m_sceneMax = sceneMax;
	m_totalObjects = (int)objects.size();

	// Build the tree recursively
	AABB2 rootBounds(sceneMin, sceneMax);
	m_root = BuildRecursive(objects, rootBounds, 0);
}

//-----------------------------------------------------------------------------------------------
void QuadTree2D::Build(std::vector<ISpatialObject2D*> const& objects, Vec2 sceneMin, Vec2 sceneMax,
	int maxObjectsPerLeaf, int maxTreeDepth)
{
	m_maxObjectsPerLeaf = maxObjectsPerLeaf;
	m_maxTreeDepth = maxTreeDepth;

	Build(objects, sceneMin, sceneMax);
}

//-----------------------------------------------------------------------------------------------
void QuadTree2D::Clear()
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
void QuadTree2D::QueryRay(Vec2 const& rayStart, Vec2 const& rayDir, float maxDist,
	std::vector<ISpatialObject2D*>& outCandidates) const
{
	outCandidates.clear();

	if (!m_root)
		return;

	QueryRayRecursive(m_root, rayStart, rayDir, maxDist, outCandidates);
}

//-----------------------------------------------------------------------------------------------
void QuadTree2D::QueryAABB(AABB2 const& bounds, std::vector<ISpatialObject2D*>& outCandidates) const
{
	outCandidates.clear();

	if (!m_root)
		return;

	QueryAABBRecursive(m_root, bounds, outCandidates);
}

//-----------------------------------------------------------------------------------------------
void QuadTree2D::QueryPoint(Vec2 const& point, std::vector<ISpatialObject2D*>& outCandidates) const
{
	outCandidates.clear();

	if (!m_root)
		return;

	QueryPointRecursive(m_root, point, outCandidates);
}

//-----------------------------------------------------------------------------------------------
AABB2 QuadTree2D::GetRootBounds() const
{
	if (m_root)
		return m_root->m_bounds;
	return AABB2();
}

//-----------------------------------------------------------------------------------------------
void QuadTree2D::DebugRender(int maxDepthToShow) const
{
	if (!m_root)
		return;

	int maxDepth = (maxDepthToShow < 0) ? m_maxDepth : maxDepthToShow;
	DebugRenderRecursive(m_root, 0, maxDepth);
}

void QuadTree2D::DebugRenderWithObjectColoring(int maxDepthToShow) const
{
	if (!m_root)
		return;

	int maxDepth = (maxDepthToShow < 0) ? m_maxDepth : maxDepthToShow;
	DebugRenderRecursiveWithColoring(m_root, 0, maxDepth);
}

//-----------------------------------------------------------------------------------------------
// PRIVATE METHODS
//-----------------------------------------------------------------------------------------------

QuadTreeNode* QuadTree2D::BuildRecursive(std::vector<ISpatialObject2D*> const& objects,
	AABB2 const& bounds, int depth)
{
	if (objects.empty())
		return nullptr;

	// Create node
	QuadTreeNode* node = new QuadTreeNode();
	node->m_bounds = bounds;
	m_nodeCount++;

	if (depth > m_maxDepth)
		m_maxDepth = depth;

	// ===== 终止条件 =====
	Vec2 extent = bounds.GetDimensions();
	float minSize = 1.0f;
	bool tooSmall = (extent.x < minSize || extent.y < minSize);

	bool shouldMakeLeaf = ((int)objects.size() <= m_maxObjectsPerLeaf) ||
		(depth >= m_maxTreeDepth) ||
		tooSmall;

	if (shouldMakeLeaf)
	{
		// 叶子节点：存储所有对象
		node->m_objects = objects;
		return node;
	}

	// ===== 细分 =====
	AABB2 childBounds[4];
	SubdivideNode(node, childBounds);

	// 分类对象
	std::vector<ISpatialObject2D*> childObjects[4];
	std::vector<ISpatialObject2D*> stayHereObjects;  // 留在当前节点的对象

	for (int i = 0; i < (int)objects.size(); ++i)
	{
		ISpatialObject2D* obj = objects[i];
		AABB2 objBounds = obj->GetBounds();

		// 检查是否有某个子象限能完全包含该对象
		bool foundContainingChild = false;

		for (int quadrant = 0; quadrant < 4; ++quadrant)
		{
			// 检查子象限是否完全包含对象
			bool fullyContained = (objBounds.m_mins.x >= childBounds[quadrant].m_mins.x) &&
				(objBounds.m_mins.y >= childBounds[quadrant].m_mins.y) &&
				(objBounds.m_maxs.x <= childBounds[quadrant].m_maxs.x) &&
				(objBounds.m_maxs.y <= childBounds[quadrant].m_maxs.y);

			if (fullyContained)
			{
				// 找到能完全包含它的子象限，向下传递
				childObjects[quadrant].push_back(obj);
				foundContainingChild = true;
				break;  // 只会被一个子象限完全包含
			}
		}

		// 如果没有子象限能完全包含它，则存储在当前节点
		if (!foundContainingChild)
		{
			stayHereObjects.push_back(obj);
		}
	}

	// ===== 检查是否值得分割 =====
	int totalChildObjects = 0;
	for (int i = 0; i < 4; ++i)
	{
		totalChildObjects += (int)childObjects[i].size();
	}

	// 如果没有对象能被传递到子节点，当前节点变成叶子
	if (totalChildObjects == 0)
	{
		node->m_objects = objects;
		return node;
	}

	// ===== 当前节点存储那些无法被子节点完全包含的对象 =====
	node->m_objects = stayHereObjects;

	// ===== 递归构建子节点 =====
	for (int i = 0; i < 4; ++i)
	{
		if (!childObjects[i].empty())
		{
			node->m_children[i] = BuildRecursive(childObjects[i], childBounds[i], depth + 1);
		}
	}

	return node;
}

//-----------------------------------------------------------------------------------------------
void QuadTree2D::SubdivideNode(QuadTreeNode* node, AABB2 childBounds[4]) const
{
	Vec2 center = node->m_bounds.GetCenter();
	Vec2 min = node->m_bounds.m_mins;
	Vec2 max = node->m_bounds.m_maxs;

	// North-East (top-right)
	childBounds[QuadTreeNode::CHILD_NE] = AABB2(
		Vec2(center.x, center.y),
		Vec2(max.x, max.y)
	);

	// North-West (top-left)
	childBounds[QuadTreeNode::CHILD_NW] = AABB2(
		Vec2(min.x, center.y),
		Vec2(center.x, max.y)
	);

	// South-West (bottom-left)
	childBounds[QuadTreeNode::CHILD_SW] = AABB2(
		Vec2(min.x, min.y),
		Vec2(center.x, center.y)
	);

	// South-East (bottom-right)
	childBounds[QuadTreeNode::CHILD_SE] = AABB2(
		Vec2(center.x, min.y),
		Vec2(max.x, center.y)
	);
}

//-----------------------------------------------------------------------------------------------
int QuadTree2D::ClassifyObject(ISpatialObject2D* obj, AABB2 const& quadrant) const
{
	AABB2 objBounds = obj->GetBounds();
	return DoAABB2Overlap(objBounds, quadrant) ? 1 : 0;
}

//-----------------------------------------------------------------------------------------------
void QuadTree2D::QueryRayRecursive(QuadTreeNode* node, Vec2 const& rayStart, Vec2 const& rayDir,
	float maxDist, std::vector<ISpatialObject2D*>& outCandidates) const
{
	if (!node)
		return;

	// Calculate ray bounding box
	Vec2 rayEnd = rayStart + rayDir * maxDist;

	AABB2 rayBounds;
	rayBounds.m_mins.x = (rayStart.x < rayEnd.x) ? rayStart.x : rayEnd.x;
	rayBounds.m_mins.y = (rayStart.y < rayEnd.y) ? rayStart.y : rayEnd.y;
	rayBounds.m_maxs.x = (rayStart.x > rayEnd.x) ? rayStart.x : rayEnd.x;
	rayBounds.m_maxs.y = (rayStart.y > rayEnd.y) ? rayStart.y : rayEnd.y;

	// Quick rejection
	if (!DoAABB2Overlap(node->m_bounds, rayBounds))
		return;

	// ===== 添加当前节点的对象（跨越边界的对象） =====
	for (int i = 0; i < (int)node->m_objects.size(); ++i)
	{
		outCandidates.push_back(node->m_objects[i]);
	}

	// ===== 递归查询子节点 =====
	if (!node->IsLeaf())
	{
		for (int i = 0; i < 4; ++i)
		{
			QueryRayRecursive(node->m_children[i], rayStart, rayDir, maxDist, outCandidates);
		}
	}
}

//-----------------------------------------------------------------------------------------------
void QuadTree2D::QueryAABBRecursive(QuadTreeNode* node, AABB2 const& bounds,
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
		// Recursively query all 4 children
		for (int i = 0; i < 4; ++i)
		{
			QueryAABBRecursive(node->m_children[i], bounds, outCandidates);
		}
	}
}

//-----------------------------------------------------------------------------------------------
void QuadTree2D::QueryPointRecursive(QuadTreeNode* node, Vec2 const& point,
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
		// Recursively query all 4 children
		for (int i = 0; i < 4; ++i)
		{
			QueryPointRecursive(node->m_children[i], point, outCandidates);
		}
	}
}

//-----------------------------------------------------------------------------------------------
void QuadTree2D::ClearRecursive(QuadTreeNode* node)
{
	if (!node)
		return;

	for (int i = 0; i < 4; ++i)
	{
		ClearRecursive(node->m_children[i]);
	}

	delete node;
}

//-----------------------------------------------------------------------------------------------
void QuadTree2D::DebugRenderRecursive(QuadTreeNode* node, int currentDepth, int maxDepth) const
{
	if (!node || currentDepth > maxDepth)
		return;

	// Draw bounds with color based on depth and leaf status
	Rgba8 color;
	if (node->IsLeaf())
	{
		color = Rgba8(255, 150, 0, 255);  // Orange for leaves
	}
	else
	{
		// Fade from cyan to dark blue as depth increases
		int intensity = 255 - (currentDepth * 25);
		if (intensity < 50) intensity = 50;
		color = Rgba8(0, intensity, 255, 255);
	}

	Vec2 min = node->m_bounds.m_mins;
	Vec2 max = node->m_bounds.m_maxs;
	std::vector<Vertex_PCU> boxVerts;
	AddVertsForAABBWire2D(boxVerts, node->m_bounds, color, 3.f, false);
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(boxVerts);

	// Recursively render all 4 children
	for (int i = 0; i < 4; ++i)
	{
		DebugRenderRecursive(node->m_children[i], currentDepth + 1, maxDepth);
	}
}

void QuadTree2D::DebugRenderRecursiveWithColoring(QuadTreeNode* node, int currentDepth, int maxDepth) const
{
	if (!node || currentDepth > maxDepth)
		return;

	// 获取当前深度的颜色
	Rgba8 nodeColor = GetColorForDepth(currentDepth);

	// ===== 1. 绘制节点边界框 =====
	std::vector<Vertex_PCU> boxVerts;
	AddVertsForAABBWire2D(boxVerts, node->m_bounds, nodeColor, 3.f, false);
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(boxVerts);

	// ===== 2. 绘制该节点存储的对象（用相同颜色） =====
	std::vector<Vertex_PCU> objectVerts;
	std::vector<Vertex_PCU> lineVerts;

	// 获取节点中心
	Vec2 nodeCenter = node->m_bounds.GetCenter();
	
	for (ISpatialObject2D* obj : node->m_objects)
	{
		Vec2 objCenter = obj->GetCenter();

		// 绘制对象中心点（圆形）
		Rgba8 centerColor = Rgba8(nodeColor.r, nodeColor.g, nodeColor.b, 100);
		AddVertsForDisc2D(objectVerts, objCenter, 6.f, centerColor);

		// 绘制从对象中心到节点中心的连线
		Rgba8 lineColor = Rgba8(nodeColor.r, nodeColor.g, nodeColor.b, 150);
		AddVertsForLineSegment2D(lineVerts, objCenter, nodeCenter, 2.f, lineColor);
	}
	AddVertsForDisc2D(lineVerts, nodeCenter, 6.f, nodeColor);
	// 绘制对象中心点
	if (!objectVerts.empty())
	{
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray(objectVerts);
	}

	// 绘制连线
	if (!lineVerts.empty())
	{
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray(lineVerts);
	}

	// ===== 3. 递归渲染子节点 =====
	for (int i = 0; i < 4; ++i)
	{
		DebugRenderRecursiveWithColoring(node->m_children[i], currentDepth + 1, maxDepth);
	}
}

Rgba8 QuadTree2D::GetColorForDepth(int depth) const
{
	static const Rgba8 depthColors[] = {
		Rgba8(255, 100, 100, 255),  // 深度0: 红色
		Rgba8(100, 255, 100, 255),  // 深度1: 绿色
		Rgba8(100, 100, 255, 255),  // 深度2: 蓝色
		Rgba8(255, 255, 100, 255),  // 深度3: 黄色
		Rgba8(255, 100, 255, 255),  // 深度4: 洋红
		Rgba8(100, 255, 255, 255),  // 深度5: 青色
		Rgba8(255, 150, 100, 255),  // 深度6: 橙色
		Rgba8(150, 100, 255, 255),  // 深度7: 紫色
		Rgba8(100, 255, 150, 255),  // 深度8: 春绿
		Rgba8(255, 100, 150, 255),  // 深度9: 粉红
	};

	int colorIndex = depth % 10;  // 循环使用颜色
	return depthColors[colorIndex];
}

AABB2 QuadTree2D::ComputeSceneBounds(std::vector<ISpatialObject2D*> const& objects) const
{
	if (objects.empty())
		return AABB2();

	// 从第一个对象开始
	AABB2 sceneBounds = objects[0]->GetBounds();

	// 扩展到包含所有对象
	for (int i = 1; i < (int)objects.size(); ++i)
	{
		AABB2 objBounds = objects[i]->GetBounds();

		sceneBounds.m_mins.x = (objBounds.m_mins.x < sceneBounds.m_mins.x) ? objBounds.m_mins.x : sceneBounds.m_mins.x;
		sceneBounds.m_mins.y = (objBounds.m_mins.y < sceneBounds.m_mins.y) ? objBounds.m_mins.y : sceneBounds.m_mins.y;
		sceneBounds.m_maxs.x = (objBounds.m_maxs.x > sceneBounds.m_maxs.x) ? objBounds.m_maxs.x : sceneBounds.m_maxs.x;
		sceneBounds.m_maxs.y = (objBounds.m_maxs.y > sceneBounds.m_maxs.y) ? objBounds.m_maxs.y : sceneBounds.m_maxs.y;
	}

	// 可选：稍微扩展一点边界，避免边界对象的浮点误差问题
	float padding = 1.0f;
	sceneBounds.m_mins.x -= padding;
	sceneBounds.m_mins.y -= padding;
	sceneBounds.m_maxs.x += padding;
	sceneBounds.m_maxs.y += padding;

	return sceneBounds;
}
