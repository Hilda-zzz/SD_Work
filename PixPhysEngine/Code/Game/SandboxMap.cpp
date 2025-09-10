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
	AddVertsForAABBWire2D(m_boundVerts, m_mapBound, Rgba8::WHITE, m_mapSize.x/100.f, true);
}

void SandboxMap::PlaceMaterial(int x, int y, CellMatType type, int brushSize)
{
	m_grid[y][x].m_type = type;
	if(type==CellMatType::MAT_SAND)
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
		UpdateWaterParticle(x, y);
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
			if (!m_grid[y][x].m_updatedThisFrame && m_grid[y][x].m_type!=CellMatType::MAT_EMPTY)
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

bool SandboxMap::TryMoveParticle(int fromX, int fromY, int toX, int toY)
{
// 	if (!IsValidPosition(toX, toY))
// 	{
// 		return false;
// 	}
// 	if (m_grid[toY][toX].m_type!=CellMatType::MAT_EMPTY)
// 	{
// 		return false;
// 	}
	
	m_grid[toY][toX] = m_grid[fromY][fromX];
	m_grid[toY][toX].m_updatedThisFrame = true;
	m_grid[fromY][fromX].SetEmpty();
	m_grid[fromY][fromX].m_color = Rgba8::WHITE;

	//m_grid[toY][toX].m_accumulatedMoveY += 1.0f;
	return true;
}

float SandboxMap::GetFrameTime()
{
	return m_curDeltaTime;
}

void SandboxMap::UpdateSandParticle(int x, int y)
{
	// rules
	// 1 drop down first
	// 2 if not empty, slide to diagno
	// 3 random l or r
// 	Cell& cell = m_grid[y][x];
// 
// 	cell.m_velocityY += GRAVITY * GetFrameTime();
// 	bool canFallDown = IsValidPosition(x, y - 1) && m_grid[y - 1][x].IsEmpty();
// 
// 	if (canFallDown) {
// 		if (cell.m_velocityY < TERMINAL_VELOCITY) {
// 			cell.m_velocityY = TERMINAL_VELOCITY;
// 		}
// 		cell.m_accumulatedMoveY += cell.m_velocityY * GetFrameTime();
// 
// 		int curY = y;
// 		while (m_grid[curY][x].m_accumulatedMoveY <= -1.0f) {
// 			if (TryMoveParticle(x, curY, x, curY - 1)) {
// 				curY -= 1;
// 			}
// 			else {
// 				m_grid[curY][x].m_velocityY = 0.0f;
// 				m_grid[curY][x].m_accumulatedMoveY = 0.0f;
// 				break;
// 			}
// 		}
// 		return;
// 	}
//  	else {
//  		if (IsValidPosition(x, y - 1) &&
//  			m_grid[y - 1][x].m_type == CellMatType::MAT_SAND) {
//  
//  			Cell& belowCell = m_grid[y - 1][x];
//  
//  			// pass the velocity (-v)
// 			bool belowIsMoving = (belowCell.m_velocityY < -0.1f) ||
// 				(belowCell.m_accumulatedMoveY < -0.1f);
// 
// 			if (belowIsMoving) {
// 				if(m_grid[y][x].m_velocityY<belowCell.m_velocityY)
// 					HandleSpeedTransfer(x, y, x, y - 1);
// 				return;
// 			}
//  		}
// 
// 
//  		// if cannot drop down, reset y velocity
//  		cell.m_velocityY = 0.0f;
//  		cell.m_accumulatedMoveY = 0.0f;
//  
//  		// try Diagonal Slide
//  		TryDiagonalSlide(x, y);
//  	}
// 
// 	Cell& cell = m_grid[y][x];
// 	cell.m_accumulatedMoveY += cell.m_velocityY * GetFrameTime();
// 	cell.m_velocityY += GRAVITY * GetFrameTime();
// 	if (cell.m_velocityY < TERMINAL_VELOCITY) {
// 		cell.m_velocityY = TERMINAL_VELOCITY;
// 	}
// 
// 	// track cur particle's pos
// 	int currentX = x;
// 	int currentY = y;
// 
// 	while (m_grid[y][x].m_accumulatedMoveY <= -1.0f) {
// 		int targetY = currentY - 1;
// 
// 		if (!IsValidPosition(currentX, targetY)) {
// // 			m_grid[currentY][currentX].m_velocityY = 0.0f;
// // 			m_grid[currentY][currentX].m_accumulatedMoveY = 0.0f;
// 			if (currentX != x || currentY != y)
// 			{
// 				m_grid[currentY][currentX] = m_grid[y][x];
// 				m_grid[y][x].SetEmpty();
// 			}
// 			break;
// 		}
// 
// 		Cell& targetCell = m_grid[targetY][currentX];
// 
// 		if (targetCell.IsEmpty()) {
// // 			if (TryMoveParticle(currentX, currentY, currentX, targetY)) {
// // 				currentY = targetY; 
// // 				m_grid[currentY][currentX].m_accumulatedMoveY += 1.0f;  
// // 			}
// // 			else {
// // 				// 移动失败，停止
// // 				m_grid[currentY][currentX].m_velocityY = 0.0f;
// // 				m_grid[currentY][currentX].m_accumulatedMoveY = 0.0f;
// // 				break;
// // 			}
// 			currentY = targetY;
// 			m_grid[y][x].m_accumulatedMoveY += 1.0f;  
// 		}
// 		else if (targetCell.m_type == CellMatType::MAT_SAND) {
// 			// 目标位置是沙子
// 			bool targetIsMoving = (targetCell.m_velocityY < -0.1f);
// 				//||(targetCell.m_accumulatedMoveY < -0.1f);
// 
// 			if (targetIsMoving) {
// 				// 目标沙子在运动，比较速度
// 				if (m_grid[y][x].m_velocityY < targetCell.m_velocityY) {
// 					// 当前粒子更快，交换速度
// 					HandleSpeedTransfer(currentX, currentY, currentX, targetY);
// 					return;
// 					// 交换后继续尝试移动（因为速度可能已经改变）
// 				}
// 			}
// 			else {
// 				// 目标沙子静止，无法向下移动，尝试侧边
// 				m_grid[currentY][currentX].m_velocityY = 0.0f;
// 				m_grid[currentY][currentX].m_accumulatedMoveY = 0.0f;
// 				TryDiagonalSlide(currentX, currentY);
// 				break;
// 			}
// 		}
// // 		else {
// // 			// 目标位置是其他固体，无法移动
// // 			m_grid[currentY][currentX].m_velocityY = 0.0f;
// // 			m_grid[currentY][currentX].m_accumulatedMoveY = 0.0f;
// // 			TryDiagonalSlide(currentX, currentY);
// // 			break;
// // 		}
// 	}
// 	if (currentX != x || currentY != y)
// 	{
// 		m_grid[currentY][currentX] = m_grid[y][x];
// 		m_grid[y][x].SetEmpty();
// 	}
// // 	if (m_grid[currentY][currentX].m_velocityY == 0.0f &&
// // 		m_grid[currentY][currentX].m_accumulatedMoveY == 0.0f) {
// // 		TryDiagonalSlide(currentX, currentY);
// // 	}

	Cell& cell = m_grid[y][x];

	//=== 1. Phys====
	float dt = GetFrameTime();
	cell.m_velocityY += GRAVITY * dt;
	cell.m_velocityY = std::min(cell.m_velocityY, TERMINAL_VELOCITY);
	cell.m_accumulatedMoveY += cell.m_velocityY * dt;
	cell.m_accumulatedMoveX += cell.m_velocityX * dt;
	if (IsValidPosition(x, y-1) && !m_grid[y-1][x].IsEmpty()) {
		cell.m_velocityY *= 0.8f; // 摩擦减速
	}

	// === 2. 基于速度的移动尝试 ===
	if (TryVelocityMove(x, y)) {
		return; // 成功移动，结束
	}

 	// === 3. 简单下降 ===
 	if (TrySimpleFall(x, y)) {
 		return; // 成功下降，结束
 	}
 
 	// === 4. 对角滑落 ===
 	if (TryDiagonalSlide(x, y)) {
 		return; // 成功滑落，结束
 	}
 
 	// === 5. 无法移动，减速 ===
 	HandleStuckParticle(x, y);

}

void SandboxMap::UpdateWaterParticle(int x, int y)
{
	Cell& cell = m_grid[y][x];
	cell.m_velocityY += GRAVITY * 0.8f * GetFrameTime();

	float waterTerminalVelocity = TERMINAL_VELOCITY * 0.7f;
	if (cell.m_velocityY < waterTerminalVelocity) {
		cell.m_velocityY = waterTerminalVelocity;
	}

	if (TryVerticalFlow(x, y)) {
		return;
	}

	if (TryDiagonalFlow(x, y)) {
		return;
	}

	TryHorizontalFlow(x, y);
}

void SandboxMap::HandleWaterFlow(int x, int y)
{
	Cell& cell = m_grid[y][x];

	// reset y velocity
	cell.m_velocityY = 0.0f;
	cell.m_accumulatedMoveY = 0.0f;

	TryWaterHozrizontalFlow(x, y);
}

void SandboxMap::TryWaterHozrizontalFlow(int x, int y)
{
	Cell& cell = m_grid[y][x];

	// diagonal
	bool canFlowDownLeft = IsValidPosition(x - 1, y - 1) && m_grid[y - 1][x - 1].IsEmpty();
	bool canFlowDownRight = IsValidPosition(x + 1, y - 1) && m_grid[y + 1][x + 1].IsEmpty();

	if (canFlowDownLeft && canFlowDownRight) {
		int direction = (rand() % 2 == 0) ? -1 : 1;
		TryMoveParticle(x, y, x + direction, y - 1);
		return;
	}
	else if (canFlowDownLeft) {
		TryMoveParticle(x, y, x - 1, y - 1);
		return;
	}
	else if (canFlowDownRight) {
		TryMoveParticle(x, y, x + 1, y - 1);
		return;
	}

	const int SPREAD_RANGE = 5;  
	const int FALL_RANGE = 2;    

	int searchDirection = (rand() % 2 == 0) ? 1 : -1;

	for (int dy = 0; dy < FALL_RANGE; dy++) {
		for (int dx = 1; dx <= SPREAD_RANGE; dx++) {
			int targetX = x + dx * searchDirection;
			int targetY = y - dy;

			if (IsValidPosition(targetX, targetY) && m_grid[targetY][targetX].IsEmpty()) {
				TryMoveParticle(x, y, targetX, targetY);
				return;
			}

			targetX = x - dx * searchDirection;
			if (IsValidPosition(targetX, targetY) && m_grid[targetY][targetX].IsEmpty()) {
				TryMoveParticle(x, y, targetX, targetY);
				return;
			}
		}
	}
}

int SandboxMap::GetEffectiveHeight(int x, int baseY)
{
	if (!IsValidPosition(x, baseY)) return 0;

	int groundLevel = 0;
	int checkY = baseY;

	// find toward +y
	while (IsValidPosition(x, checkY) &&
		!m_grid[checkY][x].IsEmpty()) {  
		groundLevel++;
		checkY++;
	}

	return groundLevel;
}

bool SandboxMap::TryVerticalFlow(int x, int y)
{
	Cell& cell = m_grid[y][x];
	bool canFallDown = IsValidPosition(x, y - 1) && m_grid[y - 1][x].IsEmpty();

	if (canFallDown) {
		cell.m_accumulatedMoveY += cell.m_velocityY * GetFrameTime();
		int curY = y;

		while (m_grid[curY][x].m_accumulatedMoveY <= -1.0f) {
			if (TryMoveParticle(x, curY, x, curY - 1)) {
				curY -= 1;
			}
		}
		return true;
	}
	else {
		if (IsValidPosition(x, y - 1) &&
			!m_grid[y - 1][x].IsEmpty()) {

			Cell& belowCell = m_grid[y - 1][x];

			// pass the velocity (-v)
			bool belowIsMoving = (belowCell.m_velocityY < -0.1f) ||
				(belowCell.m_accumulatedMoveY < -0.1f);

			if (belowIsMoving) {
				if (m_grid[y][x].m_velocityY < belowCell.m_velocityY)
					HandleSpeedTransfer(x, y, x, y - 1);
			}
			return true;
		}

		// if cannot drop down, reset y velocity
		cell.m_velocityY = 0.0f;
		cell.m_accumulatedMoveY = 0.0f;
		return false;
	}
	return false;
}

bool SandboxMap::TryDiagonalFlow(int x, int y)
{
	bool canFlowDownLeft = IsValidPosition(x - 1, y - 1) && m_grid[y - 1][x - 1].IsEmpty();
	bool canFlowDownRight = IsValidPosition(x + 1, y - 1) && m_grid[y + 1][x + 1].IsEmpty();

	if (canFlowDownLeft && canFlowDownRight) {
		int direction = (rand() % 2 == 0) ? -1 : 1;
		if (TryMoveParticle(x, y, x + direction, y - 1)) {
			Cell& newCell = m_grid[y - 1][x + direction];
			newCell.m_velocityX = direction * WATER_HORIZONTAL_SPEED;
			return true;
		}
	}
	else if (canFlowDownLeft) {
		if (TryMoveParticle(x, y, x - 1, y - 1)) {
			Cell& newCell = m_grid[y - 1][x - 1];
			newCell.m_velocityX = -WATER_HORIZONTAL_SPEED;
			return true;
		}
	}
	else if (canFlowDownRight) {
		if (TryMoveParticle(x, y, x + 1, y - 1)) {
			Cell& newCell = m_grid[y - 1][x + 1];
			newCell.m_velocityX = WATER_HORIZONTAL_SPEED;
			return true;
		}
	}

	return false;
}

void SandboxMap::TryHorizontalFlow(int x, int y)
{
	Cell& cell = m_grid[y][x];

	// 如果没有水平速度，随机给予一个方向
	if (abs(cell.m_velocityX) < 0.1f) {
		int direction = (rand() % 2 == 0) ? -1 : 1;
		cell.m_velocityX = direction * WATER_HORIZONTAL_SPEED;
	}

	// 确定移动方向
	int moveDirection = (cell.m_velocityX > 0) ? 1 : -1;
	bool canMoveHorizontal = IsValidPosition(x + moveDirection, y) &&
		m_grid[y][x + moveDirection].IsEmpty();

	if (canMoveHorizontal) {
		// 可以水平移动
		cell.m_accumulatedMoveX += cell.m_velocityX * GetFrameTime();
		int curX = x;

		while (abs(m_grid[y][curX].m_accumulatedMoveX) >= 1.0f) {
			int stepDirection = (m_grid[y][curX].m_accumulatedMoveX > 0) ? 1 : -1;
			if (TryMoveParticle(curX, y, curX + stepDirection, y)) {
				curX += stepDirection;
				m_grid[y][curX].m_accumulatedMoveX -= stepDirection;
			}
			else {
				// 移动失败，停止
				m_grid[y][curX].m_velocityX = 0.0f;
				m_grid[y][curX].m_accumulatedMoveX = 0.0f;
				break;
			}
		}
	}
	else {
		// 不能水平移动，检查侧面是什么
		if (IsValidPosition(x + moveDirection, y) &&
			!m_grid[y][x + moveDirection].IsEmpty()) {

			Cell& sideCell = m_grid[y][x + moveDirection];

			// 如果侧面是水，可以交换速度
			if (sideCell.m_type == CellMatType::MAT_WATER) {
// 				bool sideIsMoving = (abs(sideCell.m_velocityX) > 0.1f) ||
// 					(abs(sideCell.m_accumulatedMoveX) > 0.1f);

				if (abs(cell.m_velocityX) > abs(sideCell.m_velocityX)) {
					HandleHorizontalSpeedTransfer(x, y, x + moveDirection, y);
				}

// 				if (sideIsMoving) {
// 					// 检查速度是否需要交换（当前水比侧面水快）
// 				
// 				}
// 				else {
// 					// 侧面的水是静止的，当前水减速但不完全停止
// 					cell.m_velocityX *= WATER_FLOW_DAMPING;
// 					cell.m_accumulatedMoveX = 0.0f;
// 				}
			}
			else {
				// 侧面是沙子等固体，完全停止
				cell.m_velocityX = 0.0f;
				cell.m_accumulatedMoveX = 0.0f;
			}
		}
	}
}

bool SandboxMap::IsPathClear(int fromX, int fromY, int toX, int toY)
{
	if (abs(toX - fromX) <= 1 && abs(toY - fromY) <= 1) {
		return IsValidPosition(toX, toY) && m_grid[toY][toX].IsEmpty();
	}

	int dx = abs(toX - fromX);
	int dy = abs(toY - fromY);
	int steps = std::max(dx, dy);

	for (int i = 1; i <= steps; i++) {
		int checkX = fromX + (toX - fromX) * i / steps;
		int checkY = fromY + (toY - fromY) * i / steps;

		if (!IsValidPosition(checkX, checkY) || !m_grid[checkY][checkX].IsEmpty()) {
			return false;
		}
	}

	return true;
}

void SandboxMap::HandleHorizontalSpeedTransfer(int x1, int y1, int x2, int y2)
{
	Cell& fastCell = m_grid[y1][x1];
	Cell& slowCell = m_grid[y2][x2];

	std::swap(fastCell.m_velocityX, slowCell.m_velocityX);
	std::swap(fastCell.m_accumulatedMoveX, slowCell.m_accumulatedMoveX);
}

bool SandboxMap::TryVelocityMove(int x, int y)
{
// 	Cell& cell = m_grid[y][x];
// 
// 	// 计算目标位置
// 	int targetX = x + (int)cell.m_velocityX * m_curDeltaTime;
// 	int targetY = y + (int)cell.m_velocityY * m_curDeltaTime;
// 
// 	// 边界检查
// 	if (!IsValidPosition(targetX, targetY)) {
// 		return false;
// 	}
// 
// 	Cell& target = m_grid[targetY][targetX];
// 
// 	if (target.IsEmpty()) {
// 		// 目标为空，直接移动
// 		MoveParticle(x, y, targetX, targetY);
// 		return true;
// 	}
// 
// 	if (target.m_type == CellMatType::MAT_SAND && !target.m_updatedThisFrame) {
// 		// 目标是沙粒，检查是否可以推动
// 		return TryPushParticle(x, y, targetX, targetY);
// 	}
// 
// 	return false;

	Cell& cell = m_grid[y][x];

	// 检查是否有足够的累积移动距离进行移动
	if (cell.m_accumulatedMoveY > -1.0f && cell.m_accumulatedMoveX < 1.0f && cell.m_accumulatedMoveX > -1.0f) {
		return false; // 累积移动距离不够，不移动
	}

	// 计算这一帧要移动的格数
	int moveStepsY = 0;
	int moveStepsX = 0;

	// Y方向移动步数（向下为负）
	if (cell.m_accumulatedMoveY <= -1.0f) {
		moveStepsY = (int)floor(-cell.m_accumulatedMoveY); // 向下移动的格数
		moveStepsY = std::min(moveStepsY, 10); // 限制单帧最大移动距离，避免穿透
	}

	// X方向移动步数
	if (cell.m_accumulatedMoveX >= 1.0f) {
		moveStepsX = (int)floor(cell.m_accumulatedMoveX);
		moveStepsX = std::min(moveStepsX, 3);
	}
	else if (cell.m_accumulatedMoveX <= -1.0f) {
		moveStepsX = -(int)floor(-cell.m_accumulatedMoveX);
		moveStepsX = std::max(moveStepsX, -3);
	}

	// 当前位置
	int currentX = x;
	int currentY = y;

	// 实际移动的距离
	int actualMoveY = 0;
	int actualMoveX = 0;

	// === 垂直移动：逐格向下检查 ===
	for (int step = 1; step <= moveStepsY; step++) {
		int nextY = y - step; // 向下移动（Y坐标减小）

		// 边界检查
		if (!IsValidPosition(currentX, nextY)) {
			break; // 超出边界，停止移动
		}

		Cell& nextCell = m_grid[nextY][currentX];

		if (nextCell.IsEmpty()) {
			// 下方为空，可以继续向下
			actualMoveY = step;
			currentY = nextY;
		}
// 		else if (nextCell.m_type == CellMatType::MAT_SAND && !nextCell.m_updatedThisFrame) {
// 			// 遇到其他沙粒，尝试推动
// 			if (TryPushParticle(currentX, currentY, currentX, nextY)) {
// 				// 推动成功，位置已经在TryPushParticle中处理了
// 				return true;
// 			}
// 			else {
// 				// 推动失败，停止移动
// 				break;
// 			}
// 		}
		else {
			// 遇到其他障碍物，停止移动
			break;
		}
	}

	// === 水平移动：如果有水平速度且垂直移动受阻 ===
	if (moveStepsX != 0 && actualMoveY == 0) {
		int nextX = x + (moveStepsX > 0 ? 1 : -1);

		if (IsValidPosition(nextX, y) && m_grid[y][nextX].IsEmpty()) {
			currentX = nextX;
			actualMoveX = (moveStepsX > 0 ? 1 : -1);
		}
	}

	// === 执行移动 ===
	if (actualMoveY > 0 || actualMoveX != 0) {
		// 移动粒子到新位置
		MoveParticle(x, y, currentX, currentY);

		// 更新累积移动距离（扣除已移动的距离）
		Cell& movedCell = m_grid[currentY][currentX];
		movedCell.m_accumulatedMoveY += actualMoveY; // 消耗向下移动的距离

		if (actualMoveX != 0) {
			movedCell.m_accumulatedMoveX -= actualMoveX; // 消耗水平移动的距离
		}

		return true;
	}

	// === 处理被阻挡的移动 ===
		// 垂直方向：如果期望移动但实际没有移动到期望位置
	if (moveStepsY > 0 && actualMoveY < moveStepsY) {
		// 垂直方向被阻挡，重置垂直累积移动和速度
		cell.m_accumulatedMoveY = 0.0f;
		cell.m_velocityY *= 0.5f; // 碰撞减速
	}

	// 水平方向：如果期望移动但实际没有移动
	if (moveStepsX != 0 && actualMoveX == 0) {
		// 水平方向被阻挡，重置水平累积移动和速度
		cell.m_accumulatedMoveX = 0.0f;
		cell.m_velocityX *= 0.5f; // 碰撞减速
	}

	return false;
}

bool SandboxMap::TryPushParticle(int fromX, int fromY, int toX, int toY)
{
	Cell& pusher = m_grid[fromY][fromX];
	Cell& target = m_grid[toY][toX];

	// 计算速度差
	float pusherSpeed = abs(pusher.m_velocityY);
	float targetSpeed = abs(target.m_velocityY);
	float speedDiff = pusherSpeed - targetSpeed;

	// 只有速度差足够大才能推动
	if (speedDiff > SPEED_THRESHOLD) {
		// 交换位置
		SwapParticles(fromX, fromY, toX, toY);
		return true;
	}

	return false;
}

void SandboxMap::SwapParticles(int x1, int y1, int x2, int y2)
{
	Cell& cell1 = m_grid[y1][x1];
	Cell& cell2 = m_grid[y2][x2];

	// 交换粒子数据
	Cell temp = cell1;
	cell1 = cell2;
	cell2 = temp;

	// 动量传递（简化版）
	float dampingFactor = 0.9f;

	// 交换并调整速度
	std::swap(cell1.m_velocityY, cell2.m_velocityY);
	std::swap(cell1.m_velocityX, cell2.m_velocityX);

	cell1.m_velocityY *= dampingFactor;
	cell2.m_velocityY *= dampingFactor;

	// 交换累积移动
	std::swap(cell1.m_accumulatedMoveY, cell2.m_accumulatedMoveY);
	std::swap(cell1.m_accumulatedMoveX, cell2.m_accumulatedMoveX);

	// 标记已更新
	cell1.m_updatedThisFrame = true;
	cell2.m_updatedThisFrame = true;
}

void SandboxMap::MoveParticle(int fromX, int fromY, int toX, int toY)
{
	if (fromX == toX && fromY == toY) {
		return; // 没有实际移动
	}

	Cell& source = m_grid[fromY][fromX];
	Cell& target = m_grid[toY][toX];

	// 移动粒子
	target = source;
	source.SetEmpty();
}

bool SandboxMap::TrySimpleFall(int x, int y)
{
	if (!IsValidPosition(x, y - 1)) {
		return false;
	}

	Cell& below = m_grid[y - 1][x];

	if (below.IsEmpty()) {
		MoveParticle(x, y, x, y - 1);
		return true;
	}

	if (below.m_type == CellMatType::MAT_SAND && !below.m_updatedThisFrame) {
		Cell& current = m_grid[y][x];
		if (abs(current.m_velocityY) > abs(below.m_velocityY) + 1.0f) {
			SwapParticles(x, y, x, y - 1);
			return true;
		}
	}

	return false;
}

void SandboxMap::HandleStuckParticle(int x, int y)
{
	Cell& cell = m_grid[y][x];

	// 减速（摩擦效果）
	cell.m_velocityY *= 0.85f;
	cell.m_velocityX *= 0.7f;

	// 如果速度太小，完全停止
	if (abs(cell.m_velocityY) < 0.1f) {
		cell.m_velocityY = 0.0f;
	}
	if (abs(cell.m_velocityX) < 0.1f) {
		cell.m_velocityX = 0.0f;
	}
}

bool SandboxMap::TryDiagonalSlide(int x, int y)
{
// 	Cell& cell = m_grid[y][x];
// 
// 	bool canSlideLeft = IsValidPosition(x - 1, y - 1) && m_grid[y - 1][x - 1].IsEmpty();
// 	bool canSlideRight = IsValidPosition(x + 1, y - 1) && m_grid[y - 1][x + 1].IsEmpty();
// 
// 	if (canSlideLeft && canSlideRight) {
// 		int direction = (rand() % 2 == 0) ? -1 : 1;
// 		if (TryMoveParticle(x, y, x + direction, y - 1)) {
// 			Cell& newCell = m_grid[y - 1][x + direction];
// 			newCell.m_velocityY = -20.0f; 
// 		}
// 	}
// 	else if (canSlideLeft) {
// 		if (TryMoveParticle(x, y, x - 1, y - 1)) {
// 			Cell& newCell = m_grid[y - 1][x - 1];
// 			newCell.m_velocityY = -20.0f;
// 		}
// 	}
// 	else if (canSlideRight) {
// 		if (TryMoveParticle(x, y, x + 1, y - 1)) {
// 			Cell& newCell = m_grid[y - 1][x + 1];
// 			newCell.m_velocityY = -20.0f;
// 		}
// 	}
	bool leftOk = IsValidPosition(x - 1, y - 1) && m_grid[y-1][x-1].IsEmpty();
	bool rightOk = IsValidPosition(x + 1, y - 1) && m_grid[y - 1][x + 1].IsEmpty();

	if (!leftOk && !rightOk) {
		return false;
	}

	Cell& cell = m_grid[y][x];

	// 在液体中时不随机选择方向
	int targetX;
	if (leftOk && rightOk) {
		// 确定性随机选择
		int direction = (rand() % 2 == 0) ? -1 : 1;
		targetX = x + direction;
	}
	else {
		targetX = leftOk ? x - 1 : x + 1;
	}

	// 执行对角移动
	MoveParticle(x, y, targetX, y - 1);

	// 对角移动时设置适当的水平速度
	Cell& moved = m_grid[y - 1][targetX];
	moved.m_velocityX = (targetX > x) ? 1.0f : -1.0f;
	moved.m_velocityY *= 0.9f; // 轻微减速

	return true;
}

void SandboxMap::HandleSpeedTransfer(int x1, int y1, int x2, int y2)
{
	Cell& fastCell = m_grid[y1][x1];
	Cell& slowCell = m_grid[y2][x2];

	std::swap(fastCell.m_velocityY, slowCell.m_velocityY);
	std::swap(fastCell.m_accumulatedMoveY, slowCell.m_accumulatedMoveY);
}
