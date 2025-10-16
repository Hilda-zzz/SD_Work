#include "Box2DShapeBuilder.hpp"
#include <Engine/Math/MathUtils.hpp>

// 配置索引计算：
//   d(0,1) --- c(1,1)
//     |          |
//   a(0,0) --- b(1,0)
// index = a | (b<<1) | (c<<2) | (d<<3)

struct MarchingSquaresCase {
	int vertexCount;
	Vec2 vertices[6]; 
};

static const MarchingSquaresCase MARCHING_SQUARES_CASES[16] = {
	// Case 0: 0000 - 全空
	{0, {}},

	// Case 1: 0001 - 左下角
	{3, {Vec2(0.0f, 0.5f), Vec2(0.5f, 0.5f), Vec2(0.5f, 0.0f)}},

	// Case 2: 0010 - 右下角
	{3, {Vec2(0.5f, 0.0f), Vec2(0.5f, 0.5f), Vec2(1.0f, 0.5f)}},

	// Case 3: 0011 - 下边两个
	{3, {Vec2(0.0f, 0.5f), Vec2(0.5f, 0.5f), Vec2(1.0f, 0.5f)}},

	// Case 4: 0100 - 右上角
	{3, {Vec2(1.0f, 0.5f), Vec2(0.5f, 0.5f), Vec2(0.5f, 1.0f)}},

	// Case 5: 0101 - 鞍点（左下 + 右上）
	{6, {Vec2(0.0f, 0.5f), Vec2(0.5f, 0.5f), Vec2(0.5f, 0.0f),  // 左下的L
		 Vec2(0.5f, 1.0f), Vec2(0.5f, 0.5f), Vec2(1.0f, 0.5f)}}, // 右上的L

		 // Case 6: 0110 - 右边两个
		 {3, {Vec2(0.5f, 0.0f), Vec2(0.5f, 0.5f), Vec2(0.5f, 1.0f)}},

		 // Case 7: 0111 - 缺左上角
		 {3, {Vec2(0.0f, 0.5f), Vec2(0.5f, 0.5f), Vec2(0.5f, 1.0f)}},

		 // Case 8: 1000 - 左上角
		 {3, {Vec2(0.5f, 1.0f), Vec2(0.5f, 0.5f), Vec2(0.0f, 0.5f)}},

		 // Case 9: 1001 - 左边两个
		 {3, {Vec2(0.5f, 1.0f), Vec2(0.5f, 0.5f), Vec2(0.5f, 0.0f)}},

		 // Case 10: 1010 - 鞍点（右下 + 左上）
		 {6, {Vec2(0.5f, 0.0f), Vec2(0.5f, 0.5f), Vec2(1.0f, 0.5f),  // 右下的L
			  Vec2(0.0f, 0.5f), Vec2(0.5f, 0.5f), Vec2(0.5f, 1.0f)}}, // 左上的L

			  // Case 11: 1011 - 缺右上角
			  {3, {Vec2(0.5f, 1.0f), Vec2(0.5f, 0.5f), Vec2(1.0f, 0.5f)}},

			  // Case 12: 1100 - 上边两个
			  {3, {Vec2(1.0f, 0.5f), Vec2(0.5f, 0.5f), Vec2(0.0f, 0.5f)}},

			  // Case 13: 1101 - 缺右下角
			  {3, {Vec2(1.0f, 0.5f), Vec2(0.5f, 0.5f), Vec2(0.5f, 0.0f)}},

			  // Case 14: 1110 - 缺左下角
			  {3, {Vec2(0.5f, 0.0f), Vec2(0.5f, 0.5f), Vec2(0.0f, 0.5f)}},

			  // Case 15: 1111 - 全满
			  {0, {}}
};

std::vector<Vec2> Box2DShapeBuilder::ExtractOutlineFromCells(std::vector<CellWithCoords> const& cells, std::vector<Vec2>& marchingPoints)
{
	if (cells.empty()) {
		return {};
	}

	//================================
	// Get a aabb bound
	std::unordered_set<IntVec2, IntVec2Hash> occupiedCells;
	IntVec2 minPos(INT_MAX, INT_MAX);
	IntVec2 maxPos(INT_MIN, INT_MIN);

	for (auto const& cellData : cells) {
		IntVec2 pos(cellData.m_worldCoords.x, cellData.m_worldCoords.y);
		occupiedCells.insert(pos);
		minPos.x = std::min(minPos.x, pos.x);
		minPos.y = std::min(minPos.y, pos.y);
		maxPos.x = std::max(maxPos.x, pos.x);
		maxPos.y = std::max(maxPos.y, pos.y);
	}
	//==========Marching Squares======================
	std::vector<Vec2> outline;
	outline.reserve(cells.size() * 4);

	for (int gridY = minPos.y - 1; gridY <= maxPos.y; ++gridY) 
	{
		for (int gridX = minPos.x - 1; gridX <= maxPos.x; ++gridX) 
		{

			int config = GetMarchingSquaresConfig(gridX, gridY, occupiedCells);

			if (config == 0 || config == 15)  continue;

			// add outline verts
			MarchingSquaresCase const& caseData = MARCHING_SQUARES_CASES[config];

			for (int i = 0; i < caseData.vertexCount; ++i) 
			{
				Vec2 worldPos(
					(float)gridX + caseData.vertices[i].x + 0.5f,
					(float)gridY + caseData.vertices[i].y + 0.5f
				);
				outline.push_back(worldPos);
				
			}
		}
	}
	// connect outline
	if (outline.size() >= 4)
	{
		outline = ConnectAndOrderVertices(outline);
		marchingPoints.reserve(outline.size());
		for (int i = 0; i < outline.size(); i++)
		{
			marchingPoints.push_back(outline[i]);
		}

		// ================ Douglas-Peucker =================== 
		outline = ReorderOutlineForDouglasPeucker(outline);
		outline = DouglasPeucker(outline, 1.3f);
	}

	return outline;
}

Vec2 Box2DShapeBuilder::CalculateCentroid(const std::vector<Vec2>& points)
{
	if (points.empty()) 
		return Vec2(0.0f, 0.0f);

	Vec2 sum(0.0f, 0.0f);
	for (const Vec2& point : points)
	{
		sum += point;
	}
	return sum / static_cast<float>(points.size());
}

int Box2DShapeBuilder::GetMarchingSquaresConfig(int gridX, int gridY, std::unordered_set<IntVec2,IntVec2Hash> const& occupiedCells)
{
	int a = occupiedCells.count(IntVec2(gridX, gridY)) ? 1 : 0;
	int b = occupiedCells.count(IntVec2(gridX + 1, gridY)) ? 1 : 0;
	int c = occupiedCells.count(IntVec2(gridX + 1, gridY + 1)) ? 1 : 0;
	int d = occupiedCells.count(IntVec2(gridX, gridY + 1)) ? 1 : 0;

	return a | (b << 1) | (c << 2) | (d << 3);
}

std::vector<Vec2> Box2DShapeBuilder::ConnectAndOrderVertices(std::vector<Vec2> const& vertices)
{
	if (vertices.size() < 3)  return vertices;

	// 简单的贪心连接算法：每次找最近的未访问点
	std::vector<Vec2> orderedVertices;
	std::vector<bool> visited(vertices.size(), false);

	orderedVertices.reserve(vertices.size());

	// 从第一个点开始
	int currentIndex = 0;
	orderedVertices.push_back(vertices[currentIndex]);
	visited[currentIndex] = true;

	for (size_t i = 1; i < vertices.size(); ++i)
	{
		float minDist = FLT_MAX;
		int nextIndex = -1;

		// 找到距离当前点最近的未访问点
		for (size_t j = 0; j < vertices.size(); ++j)
		{
			if (!visited[j])
			{
				float dist = (vertices[j] - vertices[currentIndex]).GetLengthSquared();
				if (dist < minDist)
				{
					minDist = dist;
					nextIndex = (int)j;
				}
			}
		}

		if (nextIndex == -1) break;

		orderedVertices.push_back(vertices[nextIndex]);
		visited[nextIndex] = true;
		currentIndex = nextIndex;
	}

	return orderedVertices;
}

std::vector<Vec2> Box2DShapeBuilder::ReorderOutlineForDouglasPeucker(const std::vector<Vec2>& closedLoop)
{
	if (closedLoop.size() < 4) {
		return closedLoop;
	}

	// Find the farthest point act as start and end
	float maxDistanceSq = 0.0f;
	int splitIndex = 0;

	Vec2 centroid = CalculateCentroid(closedLoop);
	for (int i = 0; i < closedLoop.size(); ++i) 
	{
		float distSq = (closedLoop[i] - centroid).GetLengthSquared();
		if (distSq > maxDistanceSq)
		{
			maxDistanceSq = distSq;
			splitIndex = i;
		}
	}

	// reorder the vector, using the split index as the start
	std::vector<Vec2> reordered;
	for (int i = 0; i < closedLoop.size(); ++i) 
	{
		reordered.push_back(closedLoop[(splitIndex + i) % closedLoop.size()]);
	}
	reordered.push_back(reordered.front()); // make the loop close

	return reordered;
}

std::vector<Vec2> Box2DShapeBuilder::DouglasPeucker(const std::vector<Vec2>& points, float epsilon)
{
	if (points.size() < 3) return points;

	// find farthest point on the line seg
	float maxDistanceSq = 0.0f;
	int maxIndex = 0;

	Vec2 start = points.front();
	Vec2 end = points.back();

	for (int i = 1; i < points.size() - 1; ++i)
	{
		float distanceSq = PointToLineDistanceSq(points[i], start, end);
		if (distanceSq > maxDistanceSq) {
			maxDistanceSq = distanceSq;
			maxIndex = i;
		}
	}
	epsilon *= epsilon;
	// if maxDistance > epsilon -> dp
	if (maxDistanceSq > epsilon) 
	{
		// 分成两段递归处理
		std::vector<Vec2> left(points.begin(), points.begin() + maxIndex + 1);
		std::vector<Vec2> right(points.begin() + maxIndex, points.end());

		std::vector<Vec2> leftSimplified = DouglasPeucker(left, epsilon);
		std::vector<Vec2> rightSimplified = DouglasPeucker(right, epsilon);

		// 合并结果（去掉重复的中间点）
		leftSimplified.pop_back();
		leftSimplified.insert(leftSimplified.end(), rightSimplified.begin(), rightSimplified.end());
		return leftSimplified;
	}
	else 
	{
		// 所有中间点都可以省略
		return { start, end };
	}
}

float Box2DShapeBuilder::PointToLineDistanceSq(const Vec2& point, const Vec2& lineStart, const Vec2& lineEnd)
{
	Vec2 nearPoint = GetNearestPointOnInfiniteLine2D(point, lineStart, lineEnd);
	return GetDistanceSquared2D(point, nearPoint);
}

bool Box2DShapeBuilder::IsClockwise(const std::vector<Vec2>& points)
{
	float sum = 0.0f;
	for (size_t i = 0; i < points.size(); ++i) 
	{
		Vec2 p1 = points[i];
		Vec2 p2 = points[(i + 1) % points.size()];
		sum += (p2.x - p1.x) * (p2.y + p1.y);
	}
	return sum > 0.0f;
}

TriangulationOutput Box2DShapeBuilder::EarClipping(const std::vector<Vec2>& inputVertices)
{
	TriangulationOutput result;
	if (inputVertices.size() < 3) 
	{
		return result;  
	}

	// make the vertices clockwise
	std::vector<Vec2> vertices = inputVertices;
	if (!IsClockwise(vertices))
	{
		std::reverse(vertices.begin(), vertices.end());
	}
	result.vertices = vertices;

	std::vector<int> indices;
	for (int i = 0; i < vertices.size(); ++i) 
	{
		indices.push_back(i);
	}

	int attempts = 0;
	const int maxAttempts = vertices.size() * vertices.size();  // #TODO: why this number

	while (indices.size() > 3 && attempts < maxAttempts) 
	{
		bool earFound = false;

		for (int i = 0; i < indices.size(); ++i) 
		{
			int prevIndex = indices[(i - 1 + indices.size()) % indices.size()];
			int currIndex = indices[i];
			int nextIndex = indices[(i + 1) % indices.size()];

			Vec2 prev = vertices[prevIndex];
			Vec2 curr = vertices[currIndex];
			Vec2 next = vertices[nextIndex];

			// test if convex
			if (GetOrientation(prev, curr, next) > 0.0f)
			{
				continue;  // 凹顶点，不是耳朵
			}

			// Test if a point in the triangle
			bool hasPointInside = false;
			for (int j = 0; j < indices.size(); ++j) 
			{
				if (j == i || j == (i - 1 + indices.size()) % indices.size() || j == (i + 1) % indices.size()) 
				{
					continue; // jump themselves
				}

				Vec2 testPoint = vertices[indices[j]];
				if (IsPointInTriangle(testPoint, prev, curr, next)) 
				{
					hasPointInside = true;
					break;
				}
			}

			// is it is a ear, remove the index from the list
			if (!hasPointInside) 
			{
				// add to result
				result.indices.push_back(prevIndex);
				result.indices.push_back(currIndex);
				result.indices.push_back(nextIndex);
				indices.erase(indices.begin() + i);

				earFound = true;
				break;
			}
		}

		if (!earFound) 
		{
			attempts++;
		}
		else 
		{
			attempts = 0; 
		}
	}

	// add the least triangle mesh
	if (indices.size() == 3) 
	{
		result.indices.push_back(indices[0]);
		result.indices.push_back(indices[1]);
		result.indices.push_back(indices[2]);
	}
	return result;
}

float Box2DShapeBuilder::GetOrientation(const Vec2& a, const Vec2& b, const Vec2& c)
{
	return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
}

bool Box2DShapeBuilder::IsPointInTriangle(const Vec2& p, const Vec2& a, const Vec2& b, const Vec2& c)
{
	float d1 = GetOrientation(a, b, p);
	float d2 = GetOrientation(b, c, p);
	float d3 = GetOrientation(c, a, p);

	bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(hasNeg && hasPos);
}
