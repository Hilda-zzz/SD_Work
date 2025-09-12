#include "SandboxMap.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "SandboxPlayer.hpp"
extern Renderer* g_theRenderer;

SandboxMap::SandboxMap(SandboxPlayer* playerPtr, IntVec2 const& size)
	:m_player(playerPtr), m_mapSize(size), m_mapBound(AABB2(0.f,0.f,(float)size.x,(float)size.y))
{
	m_grid = std::vector(size.y, std::vector(size.x, Cell()));
	m_player->SetCurMap(this);
	Initialize();
}

void SandboxMap::Update(float deltaTime)
{
	m_curDeltaTime = deltaTime;
	m_player->Update(deltaTime);
	UpdatePhysics();
}

void SandboxMap::Render() const
{
	g_theRenderer->BeginCamera(m_player->m_camera);

	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);

	g_theRenderer->DrawVertexArray(m_boundVerts);

	std::vector<Vertex_PCU> m_cellVerts;
	for (int i = 0; i < m_mapSize.y; i++)
	{
		for (int j = 0; j < m_mapSize.x; j++)
		{
			if (m_grid[i][j].m_type != CellMatType::MAT_EMPTY)
			{
				AddVertsForAABB2D(m_cellVerts, AABB2(Vec2(j, i), Vec2(j, i) + Vec2::ONE), m_grid[i][j].m_color);
			}
		}
	}
	g_theRenderer->DrawVertexArray(m_cellVerts);

	g_theRenderer->EndCamera(m_player->m_camera);
}

void SandboxMap::Initialize()
{
	AddVertsForAABBWire2D(m_boundVerts, m_mapBound, Rgba8::WHITE, m_mapSize.x / 100.f, true);
}

void SandboxMap::PlaceMaterial(int x, int y, CellMatType type, int brushSize)
{
	m_grid[y][x].m_type = type;
	if (type == CellMatType::MAT_SAND)
		m_grid[y][x].m_color = Rgba8::HILDA;
	else
		m_grid[y][x].m_color = Rgba8::CYAN;
}

Cell& SandboxMap::GetCell(int x, int y)
{
	return m_grid[y][x];
	// TODO: insert return statement here
}

bool SandboxMap::IsValidPosition(int x, int y) const
{
	if (x >= 0 && x < m_mapSize.x && y >= 0 && y < m_mapSize.y) return true;
	return false;
}

void SandboxMap::UpdateCell(int x, int y)
{
	Cell& currentCell = m_grid[y][x];
	currentCell.m_updatedThisFrame = true;
	switch (currentCell.m_type)
	{
	case CellMatType::MAT_SAND:
		UpdateSandParticle(x, y);
		break;
	case CellMatType::MAT_WATER:
		//UpdateWaterParticle(x, y);
		break;
	default:
		break;
	}
}

void SandboxMap::UpdatePhysics()
{
	ResetUpdateFlags();

	// bot to top, left to right
	for (int y = 0; y < m_mapSize.y; y++)
	{
		for (int x = 0; x < m_mapSize.x; x++)
		{
			if (!m_grid[y][x].m_updatedThisFrame && m_grid[y][x].m_type != CellMatType::MAT_EMPTY)
			{
				UpdateCell(x, y);
			}
		}
	}
}

void SandboxMap::ResetUpdateFlags()
{
	for (int y = 0; y < m_mapSize.y; y++)
	{
		for (int x = 0; x < m_mapSize.x; x++)
		{
			m_grid[y][x].m_updatedThisFrame = false;
		}
	}
}

bool SandboxMap::IsInBounds(int x, int y)
{
	if (x >= 0 && x < m_mapSize.x && y >= 0 && y < m_mapSize.y)
	{
		return true;
	}
	return false;
}

float SandboxMap::GetFrameTime()
{
	return m_curDeltaTime;
}

void SandboxMap::UpdateSandParticle(int x, int y)
{
	Cell& cell = m_grid[y][x];

	// 物理更新 (重力向下，所以是负值)
	cell.m_velocityY += GRAVITY * m_curDeltaTime; 
	cell.m_velocityY = std::max(cell.m_velocityY, -TERMINAL_SPEED);
	cell.m_accumulMoveY += cell.m_velocityY * m_curDeltaTime;
	cell.m_accumulMoveX += cell.m_velocityX * m_curDeltaTime;

	// 检查累积移动是否足够 (向下移动，累积值为负)
	if (std::abs(cell.m_accumulMoveY) < 1.0f&& std::abs(cell.m_accumulMoveX) < 1.0f) {
		return; // 累积移动小于1，不移动
	}

	// 计算目标移动距离 (向下移动的格数)
	int moveStepsY = static_cast<int>(std::abs(cell.m_accumulMoveY));
	//cell.m_accumulMoveY += moveStepsY; // 消耗累积的移动 (因为是负值，所以加回去)

	// 沿路径逐步检查
	int currentX = x;
	int currentY = y;

	//--------------------------------------------------------------
	for (int step = 0; step < moveStepsY; step++) {
		int nextY = currentY - 1; // 向下移动 (Y轴向上，所以-1是向下)

		 // 检查是否可以直接下移
 		if (IsInBounds(currentX, nextY) && m_grid[nextY][currentX].IsEmpty()) {
 			currentY = nextY;
			cell.m_accumulMoveY += 1.f;
 			continue;
 		}
 
		// 检查下一个位置
		if (!IsInBounds(currentX, nextY) || !m_grid[nextY][currentX].IsEmpty()) {
			// 遇到阻挡，尝试斜下移动
			int directionLR = 0;
			if(cell.m_velocityX<0.2f) 
				directionLR = (rand() % 2 == 0) ? -1 : 1;
			else
				directionLR = (cell.m_velocityX < 0.f) ? -1 : 1;

			// 尝试随机方向的斜下
			if (IsInBounds(currentX + directionLR, nextY) &&
				m_grid[nextY][currentX + directionLR].IsEmpty()) {

				// 移动到斜下位置
				currentY = nextY;
				currentX = currentX + directionLR;
				cell.m_velocityX += 0.2f* cell.m_velocityY;
				cell.m_velocityY *= 0.8f;
				cell.m_accumulMoveY += 1.f;
				continue;
			}

			// 尝试另一个方向的斜下
			if (IsInBounds(currentX - directionLR, nextY) &&
				m_grid[nextY][currentX - directionLR].IsEmpty()) {
				currentY = nextY;
				currentX = currentX - directionLR;
				cell.m_velocityX += 0.2f* cell.m_velocityY;
				cell.m_velocityY *= 0.8f;
				cell.m_accumulMoveY += 1.f;
				continue;
			}

		}
		// 完全被阻挡，停止移动
		cell.m_velocityY *= 0.5f; // 碰撞减速
		break;
	}

	//--------------------------------------------------------------------------
 	// 检查累积移动是否足够 (向下移动，累积值为负)
//   	if (std::abs(cell.m_accumulMoveX) < 1.0f) {
//   		return; // 累积移动小于1，不移动
//   	}
	float directionX = (cell.m_velocityX < 0.f) ? -1.f : 1.f;
	int moveStepsX = static_cast<int>(std::abs(cell.m_accumulMoveX));

	for (int step = 0; step < moveStepsX; step++) {
		int nextX = currentX + (int)directionX;

		bool currentHasSupport = (currentY == 0) ||
			!m_grid[currentY - 1][currentX].IsEmpty();
		bool nextHasSupport = false;
		if (IsInBounds(nextX, currentY)) {
			nextHasSupport = (currentY == 0) ||
				!m_grid[currentY - 1][nextX].IsEmpty();
		}

		if (IsInBounds(nextX, currentY) &&
			currentHasSupport &&
			nextHasSupport &&
			m_grid[currentY][nextX].IsEmpty()) {

			currentX = nextX;
			cell.m_velocityX *= 0.65f;

			// 修正：根据方向正确消耗累积值
			if (cell.m_velocityX > 0) {
				cell.m_accumulMoveX -= 1.0f;  // 向右移动，减少正值
			}
			else {
				cell.m_accumulMoveX += 1.0f;  // 向左移动，减少负值（加正值）
			}
			continue;
		}

		cell.m_velocityX *= 0.5f;
		break;
	}
 	//--------------------------------------------------------------------------
	//如果成功移动了一定距离，执行最终移动
 	if (currentY != y||currentX!=x) {
 		m_grid[currentY][currentX] = m_grid[y][x];
 		m_grid[y][x].SetEmpty();
 	}
}

void SandboxMap::UpdateSandY(int x, int y)
{
}

