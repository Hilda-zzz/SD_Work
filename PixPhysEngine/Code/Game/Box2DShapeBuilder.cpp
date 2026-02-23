#include "Box2DShapeBuilder.hpp"
#include <Engine/Math/MathUtils.hpp>
#include "ThirdParty/poly2tri/sweep/sweep_context.h"
#include "ThirdParty/poly2tri/common/shapes.h"
#include "ThirdParty/poly2tri/sweep/cdt.h"
#include <unordered_map>
#include <Engine/Core/ErrorWarningAssert.hpp>
#include <queue>
#include <map>
#include <ThirdParty/tracy/tracy/Tracy.hpp>
#include "CellChunk.hpp"

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
	ZoneScoped;
	if (cells.empty()) {
		return {};
	}

	//================================

	// Get a aabb bound
	std::unordered_set<IntVec2, IntVec2Hash> occupiedCells;
	IntVec2 minPos(INT_MAX, INT_MAX);
	IntVec2 maxPos(INT_MIN, INT_MIN);

	for (auto const& cellData : cells) 
	{
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
		outline = ConnectAndOrderVerticesFast(std::move(outline));
		marchingPoints.reserve(outline.size());
		for (int i = 0; i < outline.size(); i++)
		{
			marchingPoints.push_back(outline[i]);
		}

		// ================ Douglas-Peucker =================== 
		outline = ReorderOutlineForDouglasPeucker(outline);
		outline = DouglasPeucker(outline, 2.f);
		outline.pop_back();
	}

	return outline;
}

std::vector<Vec2> Box2DShapeBuilder::ExtractOutlineFromCellsSimple(std::vector<CellWithCoords> const& cells)
{
	ZoneScoped;

	if (cells.empty()) {
		return {};
	}

	////================================
	//// 1. 构建occupied cells set和计算AABB
	////================================
	//std::unordered_set<IntVec2, IntVec2Hash> occupiedCells;
	//occupiedCells.reserve(cells.size());

	//IntVec2 minPos(INT_MAX, INT_MAX);
	//IntVec2 maxPos(INT_MIN, INT_MIN);

	//for (auto const& cellData : cells)
	//{
	//	IntVec2 pos(cellData.m_worldCoords.x, cellData.m_worldCoords.y);
	//	occupiedCells.insert(pos);

	//	minPos.x = std::min(minPos.x, pos.x);
	//	minPos.y = std::min(minPos.y, pos.y);
	//	maxPos.x = std::max(maxPos.x, pos.x);
	//	maxPos.y = std::max(maxPos.y, pos.y);
	//}

	////================================
	//// 2. Marching Squares 提取轮廓点
	////================================
	//std::vector<Vec2> outline;
	//outline.reserve(cells.size() * 4);

	//for (int gridY = minPos.y - 1; gridY <= maxPos.y; ++gridY)
	//{
	//	for (int gridX = minPos.x - 1; gridX <= maxPos.x; ++gridX)
	//	{
	//		int config = GetMarchingSquaresConfig(gridX, gridY, occupiedCells);

	//		// 跳过全空或全满的情况
	//		if (config == 0 || config == 15) {
	//			continue;
	//		}

	//		// 添加轮廓顶点
	//		MarchingSquaresCase const& caseData = MARCHING_SQUARES_CASES[config];
	//		for (int i = 0; i < caseData.vertexCount; ++i)
	//		{
	//			Vec2 worldPos(
	//				static_cast<float>(gridX) + caseData.vertices[i].x + 0.5f,
	//				static_cast<float>(gridY) + caseData.vertices[i].y + 0.5f
	//			);
	//			outline.push_back(worldPos);
	//		}
	//	}
	//}


	//--------NEW ---------
	//================================
	// 1. 计算AABB
	//================================
	IntVec2 minPos(INT_MAX, INT_MAX);
	IntVec2 maxPos(INT_MIN, INT_MIN);

	for (auto const& cellData : cells)
	{
		minPos.x = std::min(minPos.x, cellData.m_worldCoords.x);
		minPos.y = std::min(minPos.y, cellData.m_worldCoords.y);
		maxPos.x = std::max(maxPos.x, cellData.m_worldCoords.x);
		maxPos.y = std::max(maxPos.y, cellData.m_worldCoords.y);
	}

	//================================
	// 2. 构建2D grid（替代unordered_set）
	//================================
	// 扩展边界以容纳marching squares的查询范围
	int width = maxPos.x - minPos.x + 3;   // +3: -1 到 maxPos+1
	int height = maxPos.y - minPos.y + 3;

	std::vector<bool> occupiedGrid(width * height, false);

	// 填充grid（偏移1个单位，留出-1的边界）
	for (auto const& cellData : cells)
	{
		int localX = cellData.m_worldCoords.x - minPos.x + 1;
		int localY = cellData.m_worldCoords.y - minPos.y + 1;
		occupiedGrid[localY * width + localX] = true;
	}

	//================================
	// 3. Marching Squares（内联配置检查）
	//================================
	std::vector<Vec2> outline;
	outline.reserve(cells.size() * 4);

	// 遍历范围：minPos-1 到 maxPos
	for (int gridY = minPos.y - 1; gridY <= maxPos.y; ++gridY)
	{
		for (int gridX = minPos.x - 1; gridX <= maxPos.x; ++gridX)
		{
			// 内联配置检查（避免函数调用）
			int localX = gridX - minPos.x + 1;
			int localY = gridY - minPos.y + 1;
			int baseIdx = localY * width + localX;

			// 直接计算config（4次数组查询 vs 4次哈希查询）
			int config = 0;
			if (occupiedGrid[baseIdx])              config |= 1;  // bottom-left
			if (occupiedGrid[baseIdx + 1])          config |= 2;  // bottom-right
			if (occupiedGrid[baseIdx + width + 1])  config |= 4;  // top-right
			if (occupiedGrid[baseIdx + width])      config |= 8;  // top-left

			// 跳过全空或全满
			if (config == 0 || config == 15) {
				continue;
			}

			// 添加轮廓顶点
			MarchingSquaresCase const& caseData = MARCHING_SQUARES_CASES[config];
			for (int i = 0; i < caseData.vertexCount; ++i)
			{
				Vec2 worldPos(
					static_cast<float>(gridX) + caseData.vertices[i].x + 0.5f,
					static_cast<float>(gridY) + caseData.vertices[i].y + 0.5f
				);
				outline.push_back(worldPos);
			}
		}
	}


	//================================
	// 4. 连接、排序和简化轮廓
	//================================
	if (outline.size() >= 4)
	{
		// 连接并排序顶点
		outline = ConnectAndOrderVerticesFast(std::move(outline));

		// 为Douglas-Peucker算法重新排序
		outline = ReorderOutlineForDouglasPeucker(std::move(outline));

		// Douglas-Peucker简化
		outline = DouglasPeucker(std::move(outline), 2.0f);

		// 移除最后一个点（如果它是闭合轮廓的重复点）
		if (!outline.empty()) {
			outline.pop_back();
		}
	}
	else
	{
		// 如果点数太少，清空返回
		outline.clear();
	}

	return outline;  // RVO会优化掉拷贝
}

std::vector<std::vector<CellWithCoords>> Box2DShapeBuilder::SeparateConnectedComponents(const std::vector<CellWithCoords>& cells)
{
	ZoneScoped;

	std::map<std::pair<int, int>, int> coordToIndex;
	for (int i = 0; i < cells.size(); i++) 
	{
		coordToIndex[{cells[i].m_worldCoords.x, cells[i].m_worldCoords.y}] = i;
	}

	std::vector<bool> visited(cells.size(), false);
	std::vector<std::vector<CellWithCoords>> components;

	// BFS
	for (int i = 0; i < cells.size(); i++) 
	{
		if (visited[i]) continue;

		std::vector<CellWithCoords> component;
		std::queue<int> queue;
		queue.push(i);
		visited[i] = true;

		while (!queue.empty()) 
		{
			int idx = queue.front();
			queue.pop();
			component.push_back(cells[idx]);

			// check each neighbor cell
			int dx[] = { 0, 0, 1, -1 };
			int dy[] = { 1, -1, 0, 0 };

			for (int d = 0; d < 4; d++) 
			{
				int nx = cells[idx].m_worldCoords.x + dx[d];
				int ny = cells[idx].m_worldCoords.y + dy[d];

				auto it = coordToIndex.find({ nx, ny });
				if (it != coordToIndex.end()) 
				{
					int neighborIdx = it->second;
					if (!visited[neighborIdx]) 
					{
						visited[neighborIdx] = true;
						queue.push(neighborIdx);
					}
				}
			}
		}
		components.push_back(component);
	}
	return components;
}

std::vector<std::vector<CellWithCoords>> Box2DShapeBuilder::SeparateConnectedComponentsMove(std::vector<CellWithCoords>&& cells)
{
	ZoneScoped;
	if (cells.empty()) {
		return {};
	}

	constexpr int GRID_SIZE = CHUNK_SIZE * CHUNK_SIZE;  // 4096
	const size_t cellCount = cells.size();

	// 1. 堆分配 - 使用 std::vector
	std::vector<int> grid(GRID_SIZE, -1);  // 初始化为-1

	// 2. 填充grid
	int chunkBaseX = cells[0].m_worldCoords.x & ~63;
	int chunkBaseY = cells[0].m_worldCoords.y & ~63;

	for (size_t i = 0; i < cellCount; i++)
	{
		int localX = cells[i].m_worldCoords.x - chunkBaseX;
		int localY = cells[i].m_worldCoords.y - chunkBaseY;

#ifdef _DEBUG
		if (localX < 0 || localX >= CHUNK_SIZE || localY < 0 || localY >= CHUNK_SIZE) {
			DebuggerPrintf("Warning: Cell out of chunk bounds [%d, %d]\n", localX, localY);
			continue;
		}
#endif

		grid[localY * CHUNK_SIZE + localX] = static_cast<int>(i);
	}

	// 3. BFS - 堆分配
	std::vector<bool> visited(cellCount, false);  // 只需要cellCount大小
	std::vector<int> bfsQueue;
	bfsQueue.reserve(cellCount);  // 预分配，避免重新分配

	std::vector<std::vector<CellWithCoords>> components;
	components.reserve(cellCount / 64 + 1);

	for (size_t i = 0; i < cellCount; i++)
	{
		if (visited[i]) continue;

		std::vector<CellWithCoords> component;
		component.reserve(128);

		// BFS
		bfsQueue.clear();
		bfsQueue.push_back(static_cast<int>(i));
		visited[i] = true;

		size_t queueHead = 0;

		while (queueHead < bfsQueue.size())
		{
			int idx = bfsQueue[queueHead++];
			component.push_back(cells[idx]);

			int localX = cells[idx].m_worldCoords.x - chunkBaseX;
			int localY = cells[idx].m_worldCoords.y - chunkBaseY;
			int gridIdx = localY * CHUNK_SIZE + localX;

			// 上
			if (localY > 0)
			{
				int neighborIdx = grid[gridIdx - CHUNK_SIZE];
				if (neighborIdx != -1 && !visited[neighborIdx])
				{
					visited[neighborIdx] = true;
					bfsQueue.push_back(neighborIdx);
				}
			}

			// 下
			if (localY < CHUNK_SIZE - 1)
			{
				int neighborIdx = grid[gridIdx + CHUNK_SIZE];
				if (neighborIdx != -1 && !visited[neighborIdx])
				{
					visited[neighborIdx] = true;
					bfsQueue.push_back(neighborIdx);
				}
			}

			// 左
			if (localX > 0)
			{
				int neighborIdx = grid[gridIdx - 1];
				if (neighborIdx != -1 && !visited[neighborIdx])
				{
					visited[neighborIdx] = true;
					bfsQueue.push_back(neighborIdx);
				}
			}

			// 右
			if (localX < CHUNK_SIZE - 1)
			{
				int neighborIdx = grid[gridIdx + 1];
				if (neighborIdx != -1 && !visited[neighborIdx])
				{
					visited[neighborIdx] = true;
					bfsQueue.push_back(neighborIdx);
				}
			}
		}

		components.push_back(std::move(component));
	}

	return components;
}

bool Box2DShapeBuilder::IfDiscardComponent(const std::vector<CellWithCoords>& cells)
{
	const int MIN_CELLS = 40; 
	if (cells.size() < MIN_CELLS) 
	{
		return true;
	}
	return false;
}

std::vector<IntVec2> Box2DShapeBuilder::FillHoles(const std::vector<CellWithCoords>& cells)
{
	std::vector<IntVec2> result;
	if (cells.empty()) return result;

	// 1. 构建坐标到cell的映射
	std::map<std::pair<int, int>, CellWithCoords> cellMap;
	int minX = INT_MAX, maxX = INT_MIN;
	int minY = INT_MAX, maxY = INT_MIN;

	for (const auto& cell : cells) 
	{
		cellMap[{cell.m_worldCoords.x, cell.m_worldCoords.y}] = cell;
		minX = std::min(minX, cell.m_worldCoords.x);
		maxX = std::max(maxX, cell.m_worldCoords.x);
		minY = std::min(minY, cell.m_worldCoords.y);
		maxY = std::max(maxY, cell.m_worldCoords.y);
	}

	int width = maxX - minX + 1;
	int height = maxY - minY + 1;
	std::vector<std::vector<bool>> visited(
		height, std::vector<bool>(width, false));

	// 3. 从边界开始洪水填充,标记外部区域
	std::queue<std::pair<int, int>> queue;

	// 标记边界上的空白格子为外部
	auto markExternal = [&](int x, int y) 
	{
		if (x < minX || x > maxX || y < minY || y > maxY) return;
		int localX = x - minX;
		int localY = y - minY;

		if (!visited[localY][localX] &&
			cellMap.find({ x, y }) == cellMap.end()) 
		{
			visited[localY][localX] = true;
			queue.push({ x, y });
		}
	};

	// 从四条边开始
	for (int x = minX; x <= maxX; x++) 
	{
		markExternal(x, minY);
		markExternal(x, maxY);
	}
	for (int y = minY; y <= maxY; y++) 
	{
		markExternal(minX, y);
		markExternal(maxX, y);
	}

	// 4. BFS标记所有外部空格
	const int dx[] = { 0, 0, 1, -1 };
	const int dy[] = { 1, -1, 0, 0 };

	while (!queue.empty()) {
		auto [x, y] = queue.front();
		queue.pop();

		for (int i = 0; i < 4; i++) {
			int nx = x + dx[i];
			int ny = y + dy[i];

			if (nx < minX || nx > maxX || ny < minY || ny > maxY)
				continue;

			int localX = nx - minX;
			int localY = ny - minY;

			if (!visited[localY][localX] &&
				cellMap.find({ nx, ny }) == cellMap.end()) {
				visited[localY][localX] = true;
				queue.push({ nx, ny });
			}
		}
	}

	// 5. 未访问的空格就是内部空洞,需要填充
	for (int y = minY; y <= maxY; y++) 
	{
		for (int x = minX; x <= maxX; x++) 
		{
			int localX = x - minX;
			int localY = y - minY;

			// 如果是空格且未被访问(即内部空洞)
			if (cellMap.find({ x, y }) == cellMap.end() &&
				!visited[localY][localX]) 
			{
				result.push_back(IntVec2(x,y));
			}
		}
	}

	return result;
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
	ZoneScoped;
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

std::vector<Vec2> Box2DShapeBuilder::ConnectAndOrderVerticesFast(std::vector<Vec2>&& vertices)
{
	ZoneScoped;

	if (vertices.size() < 3) {
		return std::move(vertices);
	}

	const size_t vertCount = vertices.size();

	Vec2 minPos(FLT_MAX, FLT_MAX);
	Vec2 maxPos(-FLT_MAX, -FLT_MAX);

	for (auto const& v : vertices) {
		minPos.x = std::min(minPos.x, v.x);
		minPos.y = std::min(minPos.y, v.y);
		maxPos.x = std::max(maxPos.x, v.x);
		maxPos.y = std::max(maxPos.y, v.y);
	}

	// 针对Marching Squares优化：
	// 1. 轮廓点在0.5对齐的位置 (x.5, y.5)
	// 2. 相邻点距离是1.0 (直线) 或 √2 (对角)
	// 3. cellSize = 2.0 可以保证相邻点在3×3范围内
	constexpr float CELL_SIZE = 2.0f;
	constexpr float MAX_NEIGHBOR_DIST_SQ = 2.5f;  // √2 ≈ 1.414, 平方 ≈ 2.0, 留余量2.5

	int gridWidth = static_cast<int>((maxPos.x - minPos.x) / CELL_SIZE) + 2;
	int gridHeight = static_cast<int>((maxPos.y - minPos.y) / CELL_SIZE) + 2;

	struct GridCell {
		std::vector<int> indices;
	};
	std::vector<GridCell> grid(gridWidth * gridHeight);

	for (size_t i = 0; i < vertCount; ++i) {
		int gx = std::clamp(static_cast<int>((vertices[i].x - minPos.x) / CELL_SIZE), 0, gridWidth - 1);
		int gy = std::clamp(static_cast<int>((vertices[i].y - minPos.y) / CELL_SIZE), 0, gridHeight - 1);
		grid[gy * gridWidth + gx].indices.push_back(static_cast<int>(i));
	}

	std::vector<Vec2> orderedVertices;
	orderedVertices.reserve(vertCount);
	std::vector<bool> visited(vertCount, false);

	int currentIndex = 0;
	orderedVertices.push_back(vertices[currentIndex]);
	visited[currentIndex] = true;

	for (size_t i = 1; i < vertCount; ++i)
	{
		Vec2 const& currentPos = vertices[currentIndex];

		int gx = std::clamp(static_cast<int>((currentPos.x - minPos.x) / CELL_SIZE), 0, gridWidth - 1);
		int gy = std::clamp(static_cast<int>((currentPos.y - minPos.y) / CELL_SIZE), 0, gridHeight - 1);

		float minDistSq = FLT_MAX;
		int nextIndex = -1;

		// 只搜索3×3区域（radius=1）
		for (int dy = -1; dy <= 1; ++dy) {
			for (int dx = -1; dx <= 1; ++dx) {
				int nx = gx + dx;
				int ny = gy + dy;

				if (nx < 0 || nx >= gridWidth || ny < 0 || ny >= gridHeight) {
					continue;
				}

				int cellIdx = ny * gridWidth + nx;

				for (int idx : grid[cellIdx].indices) {
					if (!visited[idx]) {
						float distSq = (vertices[idx] - currentPos).GetLengthSquared();

						// 只考虑"合理距离"的点（相邻轮廓点）
						if (distSq <= MAX_NEIGHBOR_DIST_SQ && distSq < minDistSq) {
							minDistSq = distSq;
							nextIndex = idx;
						}
					}
				}
			}
		}

		// 如果3×3内没找到合理的点，说明可能有问题
		// 回退到全局搜索（但这不应该发生在正常的marching squares轮廓上）
		if (nextIndex == -1) {
#ifdef _DEBUG
			DebuggerPrintf("Warning: Grid search failed at iteration %zu, falling back to global search\n", i);
#endif

			for (size_t j = 0; j < vertCount; ++j) {
				if (!visited[j]) {
					float distSq = (vertices[j] - currentPos).GetLengthSquared();
					if (distSq < minDistSq) {
						minDistSq = distSq;
						nextIndex = static_cast<int>(j);
					}
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
	//reordered.push_back(reordered.front()); // make the loop close

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
	float e2 = epsilon * epsilon;
	// if maxDistance > epsilon -> dp
	if (maxDistanceSq > e2)
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

TriangulationOutput Box2DShapeBuilder::CDTTriangulation(std::vector<Vec2> const& points)
{
	TriangulationOutput output;

	if (points.size() < 3)
	{
		return output;
	}

	std::vector<Vec2> ccwPoints = points;
	if (IsClockwise(ccwPoints))
	{
		//std::reverse(ccwPoints.begin(), ccwPoints.end());
	}

	// 2. 转换为 poly2tri 格式
	std::vector<p2t::Point*> polyline;
	for (const auto& v : ccwPoints)
	{
		polyline.push_back(new p2t::Point(v.x, v.y));
	}

	// 3. 执行三角化
	try
	{
		p2t::CDT cdt(polyline);
		cdt.Triangulate();

		// 4. 获取三角形结果
		std::vector<p2t::Triangle*> triangles = cdt.GetTriangles();

		// 5. 构建索引映射（poly2tri 的顶点指针 -> 我们的顶点索引）
		std::unordered_map<p2t::Point*, unsigned int> pointToIndex;
		for (size_t i = 0; i < polyline.size(); ++i)
		{
			pointToIndex[polyline[i]] = static_cast<unsigned int>(i);
		}

		// 6. 填充输出
		output.vertices = ccwPoints;
		output.indices.reserve(triangles.size() * 3);

		for (auto* tri : triangles)
		{
			// poly2tri 返回的三角形可能包含内部点，我们只使用原始顶点
			for (int i = 0; i < 3; ++i)
			{
				p2t::Point* pt = tri->GetPoint(i);

				// 查找对应的原始顶点索引
				auto it = pointToIndex.find(pt);
				if (it != pointToIndex.end())
				{
					output.indices.push_back(it->second);
				}
				else
				{
					// 这是 CDT 生成的内部点（Steiner point）
					// 需要添加到顶点列表
					unsigned int newIndex = static_cast<unsigned int>(output.vertices.size());
					output.vertices.push_back(Vec2(pt->x, pt->y));
					output.indices.push_back(newIndex);
					pointToIndex[pt] = newIndex;
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		//ERROR_RECOVERABLE(Stringf("CDT 三角化失败: %s", e.what()));
		DebuggerPrintf("CDT 三角化失败: %s\n", e.what());
		// 清理内存
		for (auto* p : polyline)
		{
			delete p;
		}
		return output;
	}

	// 7. 清理内存
	for (auto* p : polyline)
	{
		delete p;
	}

	return output;
}

std::vector<IntVec2> Box2DShapeBuilder::DetectHolesToFill(const std::vector<CellWithCoords>& cells)
{
	ZoneScoped;

	if (cells.empty()) {
		return {};
	}

	// 1. 构建occupied grid
	IntVec2 minPos(INT_MAX, INT_MAX);
	IntVec2 maxPos(INT_MIN, INT_MIN);

	for (const auto& cellData : cells)
	{
		minPos.x = std::min(minPos.x, cellData.m_worldCoords.x);
		minPos.y = std::min(minPos.y, cellData.m_worldCoords.y);
		maxPos.x = std::max(maxPos.x, cellData.m_worldCoords.x);
		maxPos.y = std::max(maxPos.y, cellData.m_worldCoords.y);
	}

	int width = maxPos.x - minPos.x + 3;
	int height = maxPos.y - minPos.y + 3;

	std::vector<bool> occupiedGrid(width * height, false);

	for (const auto& cellData : cells)
	{
		int localX = cellData.m_worldCoords.x - minPos.x + 1;
		int localY = cellData.m_worldCoords.y - minPos.y + 1;
		occupiedGrid[localY * width + localX] = true;
	}

	// 2. 泛洪填充检测空洞
	std::vector<bool> visited(width * height, false);
	std::vector<IntVec2> holesToFill;

	// 从边界开始泛洪，标记外部区域
	std::queue<IntVec2> floodQueue;

	// 添加四边的起始点
	for (int x = 0; x < width; ++x)
	{
		floodQueue.push(IntVec2(x, 0));
		floodQueue.push(IntVec2(x, height - 1));
	}
	for (int y = 1; y < height - 1; ++y)
	{
		floodQueue.push(IntVec2(0, y));
		floodQueue.push(IntVec2(width - 1, y));
	}

	// BFS标记外部
	while (!floodQueue.empty())
	{
		IntVec2 pos = floodQueue.front();
		floodQueue.pop();

		int idx = pos.y * width + pos.x;

		if (pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height)
			continue;

		if (visited[idx] || occupiedGrid[idx])
			continue;

		visited[idx] = true;

		// 检查4个邻居
		floodQueue.push(IntVec2(pos.x + 1, pos.y));
		floodQueue.push(IntVec2(pos.x - 1, pos.y));
		floodQueue.push(IntVec2(pos.x, pos.y + 1));
		floodQueue.push(IntVec2(pos.x, pos.y - 1));
	}

	// 3. 未访问且未占用的区域就是空洞
	for (int y = 1; y < height - 1; ++y)
	{
		for (int x = 1; x < width - 1; ++x)
		{
			int idx = y * width + x;
			if (!visited[idx] && !occupiedGrid[idx])
			{
				// 这是一个空洞，需要填充
				IntVec2 worldCoord(x + minPos.x - 1, y + minPos.y - 1);
				holesToFill.push_back(worldCoord);
			}
		}
	}

	return holesToFill;
}

std::vector<CellWithCoords> Box2DShapeBuilder::CreateVirtualFilledCells(const std::vector<CellWithCoords>& originalCells, const std::vector<IntVec2>& holeCoords)
{
	ZoneScoped;

	// 创建虚拟的cell用于填充空洞
	// 注意：这些cell不会真正添加到地图中

	std::vector<CellWithCoords> virtualCells;
	virtualCells.reserve(holeCoords.size());

	// 使用一个静态的虚拟cell模板
	static Cell virtualCellTemplate;
	virtualCellTemplate.m_type = CellMatType::MAT_STATIC_FILL;  // 或其他合适的类型
	virtualCellTemplate.m_isBelongRb = false;  // 虚拟的，不真正属于任何rb

	for (const IntVec2& coord : holeCoords)
	{
		// 创建虚拟的CellWithCoords
		// 注意：这里的cell指针指向静态模板，不会修改实际地图
		virtualCells.emplace_back(nullptr, coord);  // nullptr表示虚拟cell
	}

	return virtualCells;
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
