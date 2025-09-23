#include "SandboxMap.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "SandboxPlayer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cmath>
#include "ThirdParty/imgui/imgui_internal.h"
#include "Engine/Window/Window.hpp"
extern Renderer* g_theRenderer;
extern Window* g_theWindow;
SandboxMap::SandboxMap(SandboxPlayer* playerPtr, IntVec2 const& size)
	:m_player(playerPtr), m_mapSize(size), m_mapBound(AABB2(0.f,0.f,(float)size.x,(float)size.y))
{
	m_grid = std::vector(size.y, std::vector(size.x, Cell()));
	m_player->SetCurMap(this);
	Initialize();
}

void SandboxMap::Update(float deltaTime)
{
	UpdateMouseGridPosition();

	m_curDeltaTime = deltaTime;
	m_player->Update(deltaTime);
	UpdatePhysics();

	UpdateStatistics();
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
				AddVertsForAABB2D(m_cellVerts, AABB2(Vec2((float)j, (float)i), Vec2((float)j, (float)i) + Vec2::ONE), m_grid[i][j].m_color);
			}
		}
	}
	g_theRenderer->DrawVertexArray(m_cellVerts);

	g_theRenderer->EndCamera(m_player->m_camera);

	RenderImGuiStats();
	RenderCellInfo();
}

void SandboxMap::RenderImGuiStats() const
{
	if (ImGui::Begin("Sandbox Statistics"))
	{
		ImGui::Text("=== Cell Statistics ===");
		ImGui::Text("Total Non-Empty Cells: %d", m_cachedNonEmptyCells);
		ImGui::Text("Total Materials Set: %d", m_totalMaterialsSet);

// 		ImGui::Separator();
// 		ImGui::Text("=== Material Breakdown ===");
// 		ImGui::Text("Sand Cells: %d", m_curMap->GetMaterialCount(CellMatType::MAT_SAND));
// 		ImGui::Text("Water Cells: %d", m_curMap->GetMaterialCount(CellMatType::MAT_WATER));
// 		ImGui::Text("Stone Cells: %d", m_curMap->GetMaterialCount(CellMatType::MAT_STONE));

// 		ImGui::Separator();
// 		ImGui::Text("=== Controls ===");
// 		ImGui::Text("Left Click: Place %s", m_isSand ? "Sand" : "Water");
// 		ImGui::Text("Right Drag: Place Stone Rectangle");
// 		ImGui::Text("C Key: Toggle Sand/Water");

		// 重置按钮
// 		if (ImGui::Button("Reset Material Counter"))
// 		{
// 			m_curMap->ResetMaterialSetCounter();
// 		}
	}
	ImGui::End();
}
void SandboxMap::RenderCellInfo() const
{
	if (ImGui::Begin("Cell Inspector"))
	{
		ImGui::Text("=== Mouse Position ===");
		ImGui::Text("Grid Position: (%d, %d)", m_mouseGridX, m_mouseGridY);

		Cell currentCell = m_grid[0][0];
		// 获取当前鼠标位置的cell信息
		if (IsInBounds(m_mouseGridX, m_mouseGridY))
		{
			currentCell = m_grid[m_mouseGridY][m_mouseGridX];
		}
		
		//if (currentCell != nullptr)
		//{
			ImGui::Separator();
			ImGui::Text("=== Cell Information ===");

			// 基本信息
			ImGui::Text("Material Type: %s", GetMaterialTypeName(currentCell.m_type));
			ImGui::Text("Is Empty: %s", currentCell.IsEmpty() ? "Yes" : "No");

			// 物理属性
			ImGui::Separator();
			ImGui::Text("=== Physics Properties ===");
			ImGui::Text("Velocity X: %.3f", currentCell.m_velocityX);
			ImGui::Text("Velocity Y: %.3f", currentCell.m_velocityY);
			ImGui::Text("Accumulated Move X: %.3f", currentCell.m_accumulMoveX);
			ImGui::Text("Accumulated Move Y: %.3f", currentCell.m_accumulMoveY);

			// 状态信息
			ImGui::Separator();
			ImGui::Text("=== State Information ===");
			ImGui::Text("Updated This Frame: %s", currentCell.m_updatedThisFrame ? "Yes" : "No");
			ImGui::Text("Is Free Falling: %s", currentCell.m_isFreeFalling ? "Yes" : "No");
			ImGui::Text("Frames Without Movement: %d", currentCell.m_framesWithoutMovement);

			// 颜色信息
			ImGui::Separator();
			ImGui::Text("=== Visual Properties ===");
			ImGui::Text("Color: R=%d G=%d B=%d A=%d",
				currentCell.m_color.r,
				currentCell.m_color.g,
				currentCell.m_color.b,
				currentCell.m_color.a);

			// 显示颜色预览
			ImVec4 colorPreview = ImVec4(
				currentCell.m_color.r / 255.0f,
				currentCell.m_color.g / 255.0f,
				currentCell.m_color.b / 255.0f,
				currentCell.m_color.a / 255.0f
			);
			ImGui::ColorEdit4("Color Preview", (float*)&colorPreview, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
// 		}
// 		else
// 		{
// 			ImGui::Separator();
// 			ImGui::Text("=== Cell Information ===");
// 			ImGui::Text("Position out of bounds or invalid");
// 		}

		// 额外的调试信息
		ImGui::Separator();
		ImGui::Text("=== Debug Info ===");
		Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
		ImGui::Text("Mouse UV: (%.3f, %.3f)", mouseUV.x, mouseUV.y);

		Vec2 mousePosInWorld = AABB2(m_player->m_camera.GetOrthoBottomLeft(), m_player->m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);
		ImGui::Text("World Position: (%.3f, %.3f)", mousePosInWorld.x, mousePosInWorld.y);
	}
	ImGui::End();
}

void SandboxMap::Initialize()
{
	AddVertsForAABBWire2D(m_boundVerts, m_mapBound, Rgba8::WHITE, m_mapSize.x / 100.f, true);
}

void SandboxMap::PlaceMaterial(int x, int y, CellMatType type, int brushSize)
{
	UNUSED(brushSize);

	if (m_grid[y][x].m_type == CellMatType::MAT_EMPTY)
	{
		m_totalMaterialsSet++;

		m_grid[y][x].m_type = type;
		if (type == CellMatType::MAT_SAND)
		{
			m_grid[y][x].m_color = Rgba8::HILDA;
			m_sandSet++;
		}
		else if (type == CellMatType::MAT_WATER)
		{
			m_grid[y][x].m_color = Rgba8::CYAN;
			m_waterSet++;
		}
		else if (type == CellMatType::MAT_STONE)
		{
			m_grid[y][x].m_color = Rgba8::BLUE;
			m_grid[y][x].m_isFreeFalling = false;
			m_stoneSet++;
		}
	}
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

void SandboxMap::UpdateStatistics()
{
	m_cachedNonEmptyCells = 0;
	m_cachedSandCells = 0;
	m_cachedWaterCells = 0;
	m_cachedStoneCells = 0;
	for (int x = 0; x < m_mapSize.x; x++) // 假设m_width是地图宽度
	{
		for (int y = 0; y < m_mapSize.y; y++) // 假设m_height是地图高度
		{
			// 根据你的实际Cell数据结构调整这部分
			if (m_grid[y][x].m_type!=CellMatType::MAT_EMPTY) // 假设有这个方法检查cell是否为空
			{
				m_cachedNonEmptyCells++;

				// 按材料类型分类统计
				CellMatType matType = m_grid[y][x].m_type; // 假设有这个方法
				switch (matType)
				{
				case CellMatType::MAT_SAND:
					m_cachedSandCells++;
					break;
				case CellMatType::MAT_WATER:
					m_cachedWaterCells++;
					break;
				case CellMatType::MAT_STONE:
					m_cachedStoneCells++;
					break;
					// 其他材料类型...
				}
			}
		}
	}
}

void SandboxMap::UpdateMouseGridPosition()
{
	Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
	Vec2 mousePosInWorld = AABB2(m_player->m_camera.GetOrthoBottomLeft(), m_player->m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);
	m_mouseGridX = static_cast<int>(floor(mousePosInWorld.x));
	m_mouseGridY = static_cast<int>(floor(mousePosInWorld.y));
}

void SandboxMap::UpdateCell(int x, int y)
{
	Cell& currentCell = m_grid[y][x];
	currentCell.m_updatedThisFrame = true;
	switch (currentCell.m_type)
	{
	case CellMatType::MAT_SAND:
		UpdateSandParticle2(x, y);
		break;
	case CellMatType::MAT_WATER:
		UpdateWaterParticle2(x, y);
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

bool SandboxMap::IsInBounds(int x, int y) const
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
 
 
 	float originalVelX = cell.m_velocityX;
 	float originalAccumX = cell.m_accumulMoveX;
 
 
 // 
 	if (cell.m_isFreeFalling)
 	{
 		// 物理更新
 		cell.m_velocityY += GRAVITY * m_curDeltaTime;
 		cell.m_velocityY = std::max(cell.m_velocityY, -TERMINAL_SPEED);
 		cell.m_accumulMoveY += cell.m_velocityY * m_curDeltaTime;
 		cell.m_accumulMoveX += cell.m_velocityX * m_curDeltaTime;
  	}
   	else
   	{
   		if (IsInBounds(x, y - 1) && (m_grid[y - 1][x].IsEmpty() || m_grid[y - 1][x].m_type == CellMatType::MAT_WATER))
   		{
   			cell.m_isFreeFalling = true;
   		}
   	}
 
 	// 限制累积移动值的绝对值上限
 	const float MAX_ACCUMULATION = 32.0f;
 	cell.m_accumulMoveY = GetClamped(cell.m_accumulMoveY, -MAX_ACCUMULATION, MAX_ACCUMULATION);
 	cell.m_accumulMoveX = GetClamped(cell.m_accumulMoveX, -MAX_ACCUMULATION, MAX_ACCUMULATION);
 
 	if (cell.m_accumulMoveX > 0.f)
 	{
 		int a = 1;
 	}
 
 	// 检查累积移动是否足够
 	if (std::abs(cell.m_accumulMoveY) < 1.0f && std::abs(cell.m_accumulMoveX) < 1.0f) {
 		return;
 	}
 
 	int targetY = y + static_cast<int>(cell.m_accumulMoveY);
 	int targetX = x + static_cast<int>(cell.m_accumulMoveX);
 
 
 	// === Bresenham移动（保留原有算法）===
 	IntVec2 currentLocation(x, y);
 	IntVec2 lastValidLocation(x, y);
 	int pathStep = 0;
 
 	auto moveCallback = [&](int pathX, int pathY) -> bool {
 		pathStep++;
 		bool isFinal = (pathX == targetX && pathY == targetY);
 		bool isFirst = (pathStep == 1);
 
 		// === 使用Java风格的统一处理函数 ===
 		bool stopped = ActOnNeighboringElement(
 			pathX, pathY,
 			isFinal,
 			isFirst,
 			currentLocation,
 			lastValidLocation,
 			0  // depth
 		);
 
 		if (!stopped) {
 			lastValidLocation.x = pathX;
 			lastValidLocation.y = pathY;
 		}
 
 		return !stopped; // Bresenham回调返回true表示继续
 	};
 
 	// 执行Bresenham移动
 	BresenhamLineExcludeStart(x, y, targetX, targetY, moveCallback);
 
 	// === 更新累积值 ===
 	UpdateAccumulatedMovement(x, y, currentLocation.x, currentLocation.y);
 
 	Cell& finalCell = m_grid[currentLocation.y][currentLocation.x];
 	if ((originalVelX > 0 && finalCell.m_accumulMoveX < -1.0f) ||
 		(originalVelX < 0 && finalCell.m_accumulMoveX > 1.0f)) {
 		printf("DIRECTION MISMATCH: originalVelX=%.2f, finalAccumX=%.2f at (%d,%d)\n",
 			originalVelX, finalCell.m_accumulMoveX, currentLocation.x, currentLocation.y);
 	}
 
//-------------------------------------------------------------------------- 
// 	 //追踪当前位置（会随着移动而更新）
//  	int currentX = x;
//  	int currentY = y;
//  	int aimStepCount = static_cast<int>(abs(cell.m_accumulMoveY)) + static_cast<int>(abs(cell.m_accumulMoveX));
//  
//  
//  	auto moveCallback = [&](int pathX, int pathY) -> bool {
//  		// 边界检查
//  		if (!IsInBounds(pathX, pathY)) {
//  			return false;
//  		}
//  
//  		Cell& targetCell = m_grid[pathY][pathX];
//  
//  
//  		if (targetCell.IsEmpty() || targetCell.m_type == CellMatType::MAT_WATER) {
//  			// 检查是否穿过水（在交换前检查）
//  			bool movingThroughWater = (targetCell.m_type == CellMatType::MAT_WATER);
//  
//  			// 执行交换：当前位置 <-> 目标位置
//  			std::swap(m_grid[currentY][currentX], m_grid[pathY][pathX]);
//  
//  			// 应用移动效果（注意：交换后粒子已在新位置）
//  			Cell& movedCell = m_grid[pathY][pathX];
//  			if (movingThroughWater) {
//  				// 穿过水时减速
//  				movedCell.m_velocityY *= 0.8f;
//  				movedCell.m_velocityX *= 0.8f;
//  			}
//  
//  			// 更新当前位置追踪
//  			currentX = pathX;
//  			currentY = pathY;
//  			aimStepCount--;
//  			return true; // 继续移动
//  		}
//  //    		else {
//  //    			// 遇到固体，停止移动
//  // 			m_grid[currentY][currentX].m_velocityY *= 0.8f;
//  // 			m_grid[currentY][currentX].m_velocityX *= 0.8f;
//  // 			return false;
//  //    		}
//  	};
//  
//  	// 执行Bresenham移动
//  	BresenhamLineExcludeStart(x, y, targetX, targetY, moveCallback);
//  
//  	// === 元胞自动机阶段：处理剩余的移动步数 ===
//  	while (aimStepCount > 0) {
//  		bool moved = false;
//  
//  		// 1. 尝试直接下落
//  		if (CanMoveTo(currentX, currentY - 1)) {
//  			std::swap(m_grid[currentY][currentX], m_grid[currentY - 1][currentX]);
//  			currentY--;
//  			aimStepCount--;
//  			moved = true;
//  		}
//  		// 2. 尝试斜下移动
//  		else {
//  			// 根据当前水平速度确定优先方向
//  			Cell& currentCell = m_grid[currentY][currentX];
//  			int primaryDir = 0;
//  			int secondaryDir = 0;
//  
//  			if (std::abs(currentCell.m_velocityX) > 0.2f) {
//  				// 有明显水平速度，按速度方向优先
//  				primaryDir = (currentCell.m_velocityX > 0) ? 1 : -1;
//  				secondaryDir = -primaryDir;
//  			}
//  			else {
//  				// 使用确定性伪随机选择
//  				int pseudoRandom = (currentX + currentY + rand()) % 2;
//  				primaryDir = pseudoRandom ? 1 : -1;
//  				secondaryDir = -primaryDir;
//  			}
//  
//  			// 尝试主要方向的斜下移动
//  			if (CanMoveTo(currentX + primaryDir, currentY - 1)) {
//  				std::swap(m_grid[currentY][currentX], m_grid[currentY - 1][currentX + primaryDir]);
//  
//  				// 更新位置
//  				currentX += primaryDir;
//  				currentY--;
//  				aimStepCount--;
//  				moved = true;
//  
//  				// 斜向移动时的速度转换
//  				Cell& movedCell = m_grid[currentY][currentX];
//  				ApplySlidingPhysics(movedCell, primaryDir);
//  			}
//  			// 尝试次要方向的斜下移动
//  			else if (CanMoveTo(currentX + secondaryDir, currentY - 1)) {
//  				std::swap(m_grid[currentY][currentX], m_grid[currentY - 1][currentX + secondaryDir]);
//  
//  				// 更新位置
//  				currentX += secondaryDir;
//  				currentY--;
//  				aimStepCount--;
//  				moved = true;
//  
//  				// 斜向移动时的速度转换
//  				Cell& movedCell = m_grid[currentY][currentX];
//  				ApplySlidingPhysics(movedCell, secondaryDir);
//  			}
//       		else {
//       			Cell& currentCell = m_grid[currentY][currentX];
//       
//       			// 检查是否有足够的水平速度进行水平移动
//       			if (std::abs(currentCell.m_velocityX) > 1.f|| std::abs(currentCell.m_velocityY) > 80.f) {
//       				//int horizontalDir = (currentCell.m_velocityX > 0) ? 1 : -1;
//       
//       				// 检查水平移动的条件
//       				if (CanMoveHorizontally(currentX, currentY, primaryDir)) {
//       					// 执行水平移动
//       					std::swap(m_grid[currentY][currentX], m_grid[currentY][currentX + primaryDir]);
//       
//       					// 更新位置
//       					currentX += primaryDir;
//       					aimStepCount--;
//       					moved = true;
//       
//       					// 应用水平移动的物理效果（传入确定的方向）
//       					Cell& movedCell = m_grid[currentY][currentX];
//       					ApplyHorizontalFriction(movedCell, primaryDir);
//       				}
//       			}
//       		}
//  		}
//  
//  		// 如果无法移动，停止元胞自动机阶段
//  		if (!moved) {
//  			// 应用碰撞效果
//  			Cell& stuckCell = m_grid[currentY][currentX];
//  			stuckCell.m_velocityY *= 0.8f; // 垂直碰撞大幅减速
//  			stuckCell.m_velocityX *= 0.8f; // 水平碰撞中等减速
//  			break;
//  		}
//  	}
//  	// === 最终更新累积值 ===
//  	Cell& finalCell = m_grid[currentY][currentX];
//  
//  	// 计算实际移动距离
//  	int actualMoveX = currentX - x;
//  	int actualMoveY = currentY - y;
//  
//  	if (actualMoveX == 0 && actualMoveY == 0)
//  	{
//  		m_grid[currentY][currentX].m_framesWithoutMovement++;
//  		if (m_grid[currentY][currentX].m_framesWithoutMovement >= 20)
//  		{
//  			m_grid[currentY][currentX].m_isFreeFalling = false;
//  			cell.m_velocityY = 0.f;
//  			cell.m_velocityY = 0.f;
//  			cell.m_accumulMoveY = 0.f;
//  			cell.m_accumulMoveX = 0.f;
//  		}
//  		return;
//  	}
//  
//  	// 保存原始累积值的符号
//  	//float originalAccumX = finalCell.m_accumulMoveX;
//  	float originalAccumY = finalCell.m_accumulMoveY;
//  
//  	// 从累积值中减去实际移动距离
//  	finalCell.m_accumulMoveX -= (float)actualMoveX;
//  	finalCell.m_accumulMoveY -= (float)actualMoveY;
//  
//  	// 检查符号是否改变，如果改变则置为0
//  	if ((originalAccumX > 0 && finalCell.m_accumulMoveX < 0) ||
//  		(originalAccumX < 0 && finalCell.m_accumulMoveX > 0)) {
//  		finalCell.m_accumulMoveX = 0.0f;
//  	}
//  
//  	if ((originalAccumY > 0 && finalCell.m_accumulMoveY < 0) ||
//  		(originalAccumY < 0 && finalCell.m_accumulMoveY > 0)) {
//  		finalCell.m_accumulMoveY = 0.0f;
//  	}
//  
//  	// 防止累积值符号错误或过小残余
//  	if (std::abs(finalCell.m_accumulMoveX) < 0.1f) finalCell.m_accumulMoveX = 0.0f;
//  	if (std::abs(finalCell.m_accumulMoveY) < 0.1f) finalCell.m_accumulMoveY = 0.0f;

}

void SandboxMap::UpdateSandY(int x, int y)
{
	UNUSED(x);
	UNUSED(y);
}

void SandboxMap::UpdateWaterParticle(int x, int y)
{
	Cell& cell = m_grid[y][x];

	// 物理更新 (重力向下，所以是负值)
	cell.m_velocityY += GRAVITY * m_curDeltaTime;
	cell.m_velocityY = std::max(cell.m_velocityY, -TERMINAL_SPEED);
	cell.m_accumulMoveY += cell.m_velocityY * m_curDeltaTime;
	cell.m_accumulMoveX += cell.m_velocityX * m_curDeltaTime;

	// 检查累积移动是否足够 (向下移动，累积值为负)
	if (std::abs(cell.m_accumulMoveY) < 1.0f && std::abs(cell.m_accumulMoveX) < 1.0f) {
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
			if (cell.m_velocityX < 0.2f)
				directionLR = (rand() % 2 == 0) ? -1 : 1;
			else
				directionLR = (cell.m_velocityX < 0.f) ? -1 : 1;

			// 尝试随机方向的斜下
			if (IsInBounds(currentX + directionLR, nextY) &&
				m_grid[nextY][currentX + directionLR].IsEmpty()) {

				// 移动到斜下位置
				currentY = nextY;
				currentX = currentX + directionLR;
				cell.m_velocityY *= 0.8f;
				cell.m_accumulMoveY += 1.f;
				continue;
			}

			// 尝试另一个方向的斜下
			if (IsInBounds(currentX - directionLR, nextY) &&
				m_grid[nextY][currentX - directionLR].IsEmpty()) {
				currentY = nextY;
				currentX = currentX - directionLR;
				cell.m_velocityY *= 0.8f;
				cell.m_accumulMoveY += 1.f;
				continue;
			}

		}
		// 完全被阻挡，停止移动
		cell.m_velocityY *= 0.5f; // 碰撞减速
		cell.m_accumulMoveY *= 0.5f;
		break;
	}

	//--------------------------------------------------------------------------
 
	int directionX = 0;
	if (cell.m_velocityX < 0.2f)
		directionX = (rand() % 2 == 0) ? -1 : 1;
	else
		directionX = (cell.m_velocityX < 0.f) ? -1 : 1;
 	int moveStepsX = 10;
 
 	for (int step = 0; step < moveStepsX; step++) {
 		int nextX = currentX + (int)directionX;
		int nextXReverse= currentX - (int)directionX;
 		bool currentHasSupport = (currentY == 0) ||
 			!m_grid[currentY - 1][currentX].IsEmpty();
 		bool nextHasSupport = false;
 		if (IsInBounds(nextX, currentY)) {
 			nextHasSupport = (currentY == 0) ||
 				!m_grid[currentY - 1][nextX].IsEmpty();
 		}

		bool nextReverseHasSupport = false;
		if (IsInBounds(nextXReverse, currentY))
		{
			nextReverseHasSupport = (currentY == 0) ||
				!m_grid[currentY - 1][nextXReverse].IsEmpty();
		}

 		if (IsInBounds(nextX, currentY) &&
 			currentHasSupport &&
 			nextHasSupport &&
 			m_grid[currentY][nextX].IsEmpty()) {
 			currentX = nextX;
 			continue;
 		}
		else if (IsInBounds(nextXReverse, currentY) &&
			currentHasSupport &&
			nextReverseHasSupport &&
			m_grid[currentY][nextXReverse].IsEmpty())
		{
			currentX = nextXReverse;
			continue;
		}
 		break;
 	}
	//--------------------------------------------------------------------------
	//如果成功移动了一定距离，执行最终移动
	if (currentY != y || currentX != x) {
		m_grid[currentY][currentX] = m_grid[y][x];
		m_grid[y][x].SetEmpty();
	}
}

void SandboxMap::UpdateSandParticle2(int x, int y)
{
	Cell& cell = m_grid[y][x];

	// 保存原始状态用于调试
	float originalVelX = cell.m_velocityX;
	float originalAccumX = cell.m_accumulMoveX;

	// === 物理更新阶段 ===
	if (cell.m_isFreeFalling) {
		// 重力和终端速度
		cell.m_velocityY += GRAVITY * m_curDeltaTime;
		cell.m_velocityY = std::max(cell.m_velocityY, -TERMINAL_SPEED);

		// 累积移动量更新
		cell.m_accumulMoveY += cell.m_velocityY * m_curDeltaTime;
		cell.m_accumulMoveX += cell.m_velocityX * m_curDeltaTime;
	}
	else {
		// 检查是否应该开始自由落体
		if (IsInBounds(x, y - 1) &&
			(m_grid[y - 1][x].IsEmpty() || m_grid[y - 1][x].m_type == CellMatType::MAT_WATER)) {
			cell.m_isFreeFalling = true;
		}
		else
			return;
	}

	// 限制累积移动值上限
	const float MAX_ACCUMULATION = 32.0f;
	cell.m_accumulMoveY = GetClamped(cell.m_accumulMoveY, -MAX_ACCUMULATION, MAX_ACCUMULATION);
	cell.m_accumulMoveX = GetClamped(cell.m_accumulMoveX, -MAX_ACCUMULATION, MAX_ACCUMULATION);

	// 检查是否需要移动
	if (std::abs(cell.m_accumulMoveY) < 1.0f && std::abs(cell.m_accumulMoveX) < 1.0f) {
		return;
	}

	// 计算目标位置和移动步数
	int targetY = y + static_cast<int>(cell.m_accumulMoveY);
	int targetX = x + static_cast<int>(cell.m_accumulMoveX);

	// 追踪当前位置（会随着移动而更新）
	int currentX = x;
	int currentY = y;

	// 计算总移动步数（使用曼哈顿距离）
	int totalSteps = std::abs(targetY - y) + std::abs(targetX - x);
	int remainingSteps = totalSteps;

	// === 阶段1: Bresenham直线移动（处理物理轨迹）===
	bool bresenhamCompleted = true;

	auto moveCallback = [&](int pathX, int pathY) -> bool {
		// 边界检查
		if (!IsInBounds(pathX, pathY)) {
			bresenhamCompleted = false;
			return false;
		}

		Cell& targetCell = m_grid[pathY][pathX];
		Cell& currentCell = m_grid[currentY][currentX];

		// 情况1: 空格子，直接移动
		if (targetCell.IsEmpty()) {
			std::swap(m_grid[currentY][currentX], m_grid[pathY][pathX]);
			currentX = pathX;
			currentY = pathY;
			remainingSteps--;

			// 保持自由落体状态
			m_grid[currentY][currentX].m_isFreeFalling = true;
			return true; // 继续移动
		}

		// 情况2: 水，可穿透但有阻力
		else if (targetCell.m_type == CellMatType::MAT_WATER) {
			std::swap(m_grid[currentY][currentX], m_grid[pathY][pathX]);
			currentX = pathX;
			currentY = pathY;
			remainingSteps--;

			// 应用水的阻力
			Cell& movedCell = m_grid[currentY][currentX];
			movedCell.m_velocityY *= 0.8f;
			movedCell.m_velocityX *= 0.8f;
			movedCell.m_isFreeFalling = true;
			return true; // 继续移动
		}

		else if (targetCell.m_type == CellMatType::MAT_SAND) {
			// 执行沙粒间碰撞处理
			bool collisionHandled = HandleSandCollision(
				currentX, currentY,    // 当前沙粒位置
				pathX, pathY,          // 目标沙粒位置
				currentCell, targetCell
			);
		}

		// 情况3: 固体，应用碰撞物理并停止Bresenham
		else {
			ApplyCollisionPhysics(currentCell, pathX - currentX, pathY - currentY);
			bresenhamCompleted = false;
			return false; // 停止Bresenham移动
		}
		};

	// 执行Bresenham移动
	BresenhamLineExcludeStart(x, y, targetX, targetY, moveCallback);

	// === 阶段2: 元胞自动机移动（处理剩余移动和沙粒流动行为）===
	int maxCellularSteps = remainingSteps + 5; // 防止无限循环
	//int maxCellularSteps = 1;
	int cellularSteps = 0;

	while (remainingSteps > 0 && cellularSteps < maxCellularSteps) {
		cellularSteps++;
		bool moved = false;

		Cell& currentCell = m_grid[currentY][currentX];

		// 1. 优先尝试垂直下落
		if (CanMoveTo(currentX, currentY - 1)) {
			std::swap(m_grid[currentY][currentX], m_grid[currentY - 1][currentX]);
			currentY--;
			remainingSteps--;
			moved = true;

			// 保持下落状态
			m_grid[currentY][currentX].m_isFreeFalling = true;
		}

		// 2. 尝试斜向下落
		else {
			int primaryDir, secondaryDir;
			GetMovementDirections(currentCell, currentX, currentY, primaryDir, secondaryDir);

			// 尝试主要方向的斜下移动
			if (CanMoveTo(currentX + primaryDir, currentY - 1)) {
				std::swap(m_grid[currentY][currentX], m_grid[currentY - 1][currentX + primaryDir]);
				currentX += primaryDir;
				currentY--;
				remainingSteps--;
				moved = true;

				// 应用斜向移动的物理效果
				Cell& movedCell = m_grid[currentY][currentX];
				ApplySlidingPhysics(movedCell, primaryDir);
			}

			// 3. 如果垂直和斜向都无法移动，尝试水平移动
			if (CanMoveHorizontally(currentX, currentY, currentCell) && abs(currentCell.m_velocityX) > 2.f) {
				int horizontalDir = currentCell.m_velocityX > 0.f ? 1 : -1;

				if (CanMoveTo(currentX + horizontalDir, currentY)) {
					std::swap(m_grid[currentY][currentX], m_grid[currentY][currentX + horizontalDir]);
					currentX += horizontalDir;
					remainingSteps--;
					moved = true;

					// 应用水平移动的物理效果
					Cell& movedCell = m_grid[currentY][currentX];
					ApplyHorizontalMovementPhysics(movedCell, horizontalDir);
				}
			}
		}

		// 如果无法移动，停止元胞自动机阶段
		if (!moved) {
			ApplyStuckPhysics(m_grid[currentY][currentX]);
			break;
		}
	}

	// === 最终更新累积移动量 ===
	UpdateAccumulatedMovement(x, y, currentX, currentY);

	// 调试：检查方向不匹配
	Cell& finalCell = m_grid[currentY][currentX];
	if ((originalVelX > 0 && finalCell.m_accumulMoveX < -1.0f) ||
		(originalVelX < 0 && finalCell.m_accumulMoveX > 1.0f)) {
		printf("DIRECTION MISMATCH: originalVelX=%.2f, finalAccumX=%.2f at (%d,%d)\n",
			originalVelX, finalCell.m_accumulMoveX, currentX, currentY);
	}
}

void SandboxMap::UpdateWaterParticle2(int x, int y)
{
	Cell& cell = m_grid[y][x];

	// 保存原始状态用于调试
	float originalVelX = cell.m_velocityX;
	float originalAccumX = cell.m_accumulMoveX;

	// === 物理更新阶段 ===
	// 水总是受重力影响
	cell.m_velocityY += GRAVITY * m_curDeltaTime;
	cell.m_velocityY = std::max(cell.m_velocityY, -TERMINAL_SPEED * 1.2f); // 水的终端速度稍高
	//cell.m_velocityX = std::max(cell.m_velocityX, -TERMINAL_SPEED * 1.2f);
	// 累积移动量更新
	cell.m_accumulMoveY += cell.m_velocityY * m_curDeltaTime;
	cell.m_accumulMoveX += cell.m_velocityX * m_curDeltaTime;

	// 限制累积移动值上限
	const float MAX_ACCUMULATION = 32.0f;
	cell.m_accumulMoveY = GetClamped(cell.m_accumulMoveY, -MAX_ACCUMULATION, MAX_ACCUMULATION/2);
	cell.m_accumulMoveX = GetClamped(cell.m_accumulMoveX, -MAX_ACCUMULATION, MAX_ACCUMULATION/2);

	// 检查是否需要移动
	if (std::abs(cell.m_accumulMoveY) < 1.0f && std::abs(cell.m_accumulMoveX) < 1.0f) {
		return;
	}

	// 计算目标位置和移动步数
	int targetY = y + static_cast<int>(cell.m_accumulMoveY);
	int targetX = x + static_cast<int>(cell.m_accumulMoveX);

	// 追踪当前位置
	int currentX = x;
	int currentY = y;

	// 计算总移动步数
	int totalSteps = std::abs(targetY - y) + std::abs(targetX - x);
	int remainingSteps = totalSteps;

	// === 阶段1: Bresenham直线移动（处理物理轨迹）===
	bool bresenhamCompleted = true;

	auto moveCallback = [&](int pathX, int pathY) -> bool {
		// 边界检查
		if (!IsInBounds(pathX, pathY)) {
			bresenhamCompleted = false;
			return false;
		}

		Cell& targetCell = m_grid[pathY][pathX];
		Cell& currentCell = m_grid[currentY][currentX];

		// 情况1: 空格子，直接移动
		if (targetCell.IsEmpty()) {
			std::swap(m_grid[currentY][currentX], m_grid[pathY][pathX]);
			currentX = pathX;
			currentY = pathY;
			remainingSteps--;
			return true; // 继续移动
		}

		// 情况2: 水与水合并（密度效应）
  		else if (targetCell.m_type == CellMatType::MAT_WATER) {

 			float absY = std::abs(cell.m_velocityY);
 			float transferMomentum = absY * 0.1f; // 比沙子转换更多
 			//cell.m_velocityY *= 0.5f;
 			if (std::abs(cell.m_velocityX) > 0.1f) {
 				cell.m_velocityX += (cell.m_velocityX > 0) ? transferMomentum : -transferMomentum;
 			}
  		}

		// 情况3: 遇到沙子，尝试推动或绕过
//   		else if (targetCell.m_type == CellMatType::MAT_SAND) {
//   			// 水可以推动一些沙子（如果沙子不稳定）
//   			if (!targetCell.m_isFreeFalling &&
//   				std::sqrt(currentCell.m_velocityX * currentCell.m_velocityX +
//   					currentCell.m_velocityY * currentCell.m_velocityY) > 4.0f) {
//   
//   				// 尝试激活沙粒
//   				float activationChance = GetWaterRandom(currentX, currentY, pathX, pathY);
//   				if (activationChance < 0.3f) { // 30%概率
//   					targetCell.m_isFreeFalling = true;
//   					targetCell.m_velocityX += currentCell.m_velocityX * 0.2f;
//   					targetCell.m_velocityY += currentCell.m_velocityY * 0.2f;
//   				}
//   			}
//   
//   			// 应用碰撞物理并停止
//   			ApplyWaterCollisionPhysics(currentCell, pathX - currentX, pathY - currentY);
//   			bresenhamCompleted = false;
//   			return false;
//   		}

		// 情况4: 其他固体，应用碰撞物理并停止
		else {
			ApplyWaterCollisionPhysics(currentCell, pathX - currentX, pathY - currentY);
			bresenhamCompleted = false;
			return false;
		}
	};

	// 执行Bresenham移动
	BresenhamLineExcludeStart(x, y, targetX, targetY, moveCallback);

	// === 阶段2: 水的流动行为（元胞自动机）===
	int maxCellularSteps = remainingSteps + 30; // 水比沙子更流动
	int cellularSteps = 0;

	//while(true){
	while (remainingSteps > 0 && cellularSteps < maxCellularSteps) {
	//while (cellularSteps < maxCellularSteps) {
		cellularSteps++;
		bool moved = false;

		Cell& currentCell = m_grid[currentY][currentX];

		// 1. 优先尝试垂直下落
		if (CanMoveTo(currentX, currentY - 1)) {
			std::swap(m_grid[currentY][currentX], m_grid[currentY - 1][currentX]);
			currentY--;
			remainingSteps--;
			moved = true;
		}

		// 2. 尝试斜向下落
		else {
			int primaryDir, secondaryDir;
			GetWaterMovementDirections(currentCell, currentX, currentY, primaryDir, secondaryDir);

			// 尝试主要方向的斜下移动
			if (CanMoveTo(currentX + primaryDir, currentY - 1)) {
				std::swap(m_grid[currentY][currentX], m_grid[currentY - 1][currentX + primaryDir]);
				currentX += primaryDir;
				currentY--;
				remainingSteps--;
				moved = true;

				// 斜向流动时的速度调整
				Cell& movedCell = m_grid[currentY][currentX];
				movedCell.m_velocityX += primaryDir * 0.5f; // 增强水平流动
				//movedCell.m_velocityY *= 0.95f; // 轻微减速
			}
			// 尝试次要方向的斜下移动
 			else if (CanMoveTo(currentX + secondaryDir, currentY - 1)) {
 				std::swap(m_grid[currentY][currentX], m_grid[currentY - 1][currentX + secondaryDir]);
 				currentX += secondaryDir;
 				currentY--;
 				remainingSteps--;
 				moved = true;
 
 				Cell& movedCell = m_grid[currentY][currentX];
 				movedCell.m_velocityX += secondaryDir * 0.5f;
 				//movedCell.m_velocityY *= 0.95f;
 			}

			// 3. 水平流动（水的特殊行为）
			else if (HasSupport(currentX, currentY)) {

				int flowDirection = GetWaterFlowDirection(currentCell, currentX, currentY);

				if (CanMoveTo(currentX + flowDirection, currentY)) {
					std::swap(m_grid[currentY][currentX], m_grid[currentY][currentX + flowDirection]);
					currentX += flowDirection;
					remainingSteps--;
					moved = true;

					// 水平流动时增强水平速度
					Cell& movedCell = m_grid[currentY][currentX];
					movedCell.m_velocityX += flowDirection * 1.0f;
					//movedCell.m_velocityX *= 0.9f; // 轻微摩擦
				}
			}
		}

		// 如果无法移动，停止流动
		if (!moved) {
			//ApplyWaterStuckPhysics(m_grid[currentY][currentX]);
			break;
		}
	}

	// === 最终更新累积移动量 ===
	UpdateAccumulatedMovement(x, y, currentX, currentY);
}
void SandboxMap::UpdateWaterParticle3(int x, int y)
{
	Cell& cell = m_grid[y][x];

	// 保存原始状态用于调试
	float originalVelX = cell.m_velocityX;
	float originalAccumX = cell.m_accumulMoveX;

	// === 物理更新阶段 ===
	if (cell.m_isFreeFalling) {
		// 重力和终端速度
		cell.m_velocityY += GRAVITY * m_curDeltaTime;
		cell.m_velocityY = std::max(cell.m_velocityY, -TERMINAL_SPEED);

		// 累积移动量更新
		cell.m_accumulMoveY += cell.m_velocityY * m_curDeltaTime;
		cell.m_accumulMoveX += cell.m_velocityX * m_curDeltaTime;
	}
	else {
		// 检查是否应该开始自由落体
		if (IsInBounds(x, y - 1) &&
			(m_grid[y - 1][x].IsEmpty() || m_grid[y - 1][x].m_type == CellMatType::MAT_WATER)) {
			cell.m_isFreeFalling = true;
		}
		else
			return;
	}

	// 限制累积移动值上限
	const float MAX_ACCUMULATION = 32.0f;
	cell.m_accumulMoveY = GetClamped(cell.m_accumulMoveY, -MAX_ACCUMULATION, MAX_ACCUMULATION);
	cell.m_accumulMoveX = GetClamped(cell.m_accumulMoveX, -MAX_ACCUMULATION, MAX_ACCUMULATION);

	// 检查是否需要移动
	if (std::abs(cell.m_accumulMoveY) < 1.0f && std::abs(cell.m_accumulMoveX) < 1.0f) {
		return;
	}

	// 计算目标位置和移动步数
	int targetY = y + static_cast<int>(cell.m_accumulMoveY);
	int targetX = x + static_cast<int>(cell.m_accumulMoveX);

	// 追踪当前位置（会随着移动而更新）
	int currentX = x;
	int currentY = y;

	// 计算总移动步数（使用曼哈顿距离）
	int totalSteps = std::abs(targetY - y) + std::abs(targetX - x);
	int remainingSteps = totalSteps;

	// === 阶段1: Bresenham直线移动（处理物理轨迹）===
	bool bresenhamCompleted = true;

	auto moveCallback = [&](int pathX, int pathY) -> bool {
		// 边界检查
		if (!IsInBounds(pathX, pathY)) {
			bresenhamCompleted = false;
			return false;
		}

		Cell& targetCell = m_grid[pathY][pathX];
		Cell& currentCell = m_grid[currentY][currentX];

		// 情况1: 空格子，直接移动
		if (targetCell.IsEmpty()) {
			std::swap(m_grid[currentY][currentX], m_grid[pathY][pathX]);
			currentX = pathX;
			currentY = pathY;
			remainingSteps--;

			// 保持自由落体状态
			m_grid[currentY][currentX].m_isFreeFalling = true;
			return true; // 继续移动
		}

		// 情况2: 水，可穿透但有阻力
		else if (targetCell.m_type == CellMatType::MAT_WATER) {
			std::swap(m_grid[currentY][currentX], m_grid[pathY][pathX]);
			currentX = pathX;
			currentY = pathY;
			remainingSteps--;

			// 应用水的阻力
			Cell& movedCell = m_grid[currentY][currentX];
			movedCell.m_velocityY *= 0.8f;
			movedCell.m_velocityX *= 0.8f;
			movedCell.m_isFreeFalling = true;
			return true; // 继续移动
		}

		else if (targetCell.m_type == CellMatType::MAT_SAND) {
			// 执行沙粒间碰撞处理
			bool collisionHandled = HandleSandCollision(
				currentX, currentY,    // 当前沙粒位置
				pathX, pathY,          // 目标沙粒位置
				currentCell, targetCell
			);
		}

		// 情况3: 固体，应用碰撞物理并停止Bresenham
		else {
			ApplyCollisionPhysics(currentCell, pathX - currentX, pathY - currentY);
			bresenhamCompleted = false;
			return false; // 停止Bresenham移动
		}
		};

	// 执行Bresenham移动
	BresenhamLineExcludeStart(x, y, targetX, targetY, moveCallback);

	// === 阶段2: 元胞自动机移动（处理剩余移动和沙粒流动行为）===
	int maxCellularSteps = remainingSteps + 5; // 防止无限循环
	//int maxCellularSteps = 1;
	int cellularSteps = 0;

	//while(true){
	while (remainingSteps > 0 && cellularSteps < maxCellularSteps) {
		cellularSteps++;
		bool moved = false;

		Cell& currentCell = m_grid[currentY][currentX];

		// 1. 优先尝试垂直下落
		if (CanMoveTo(currentX, currentY - 1)) {
			std::swap(m_grid[currentY][currentX], m_grid[currentY - 1][currentX]);
			currentY--;
			remainingSteps--;
			moved = true;

			// 保持下落状态
			m_grid[currentY][currentX].m_isFreeFalling = true;
		}

		// 2. 尝试斜向下落
		else {
			int primaryDir, secondaryDir;
			GetMovementDirections(currentCell, currentX, currentY, primaryDir, secondaryDir);

			// 尝试主要方向的斜下移动
			if (CanMoveTo(currentX + primaryDir, currentY - 1)) {
				std::swap(m_grid[currentY][currentX], m_grid[currentY - 1][currentX + primaryDir]);
				currentX += primaryDir;
				currentY--;
				remainingSteps--;
				moved = true;

				// 应用斜向移动的物理效果
				Cell& movedCell = m_grid[currentY][currentX];
				ApplySlidingPhysics(movedCell, primaryDir);
			}

			// 3. 如果垂直和斜向都无法移动，尝试水平移动
			if (CanMoveHorizontally(currentX, currentY, currentCell) && abs(currentCell.m_velocityX) > 2.f) {
				int horizontalDir = currentCell.m_velocityX > 0.f ? 1 : -1;

				if (CanMoveTo(currentX + horizontalDir, currentY)) {
					std::swap(m_grid[currentY][currentX], m_grid[currentY][currentX + horizontalDir]);
					currentX += horizontalDir;
					remainingSteps--;
					moved = true;

					// 应用水平移动的物理效果
					Cell& movedCell = m_grid[currentY][currentX];
					ApplyHorizontalMovementPhysics(movedCell, horizontalDir);
				}
			}
		}

		// 如果无法移动，停止元胞自动机阶段
		if (!moved) {
			ApplyStuckPhysics(m_grid[currentY][currentX]);
			break;
		}
	}

	// === 最终更新累积移动量 ===
	UpdateAccumulatedMovement(x, y, currentX, currentY);

	// 调试：检查方向不匹配
	Cell& finalCell = m_grid[currentY][currentX];
	if ((originalVelX > 0 && finalCell.m_accumulMoveX < -1.0f) ||
		(originalVelX < 0 && finalCell.m_accumulMoveX > 1.0f)) {
		printf("DIRECTION MISMATCH: originalVelX=%.2f, finalAccumX=%.2f at (%d,%d)\n",
			originalVelX, finalCell.m_accumulMoveX, currentX, currentY);
	}
}
void SandboxMap::GetWaterMovementDirections(const Cell& cell, int x, int y, int& primaryDir, int& secondaryDir)
{
	if (std::abs(cell.m_velocityX) > 0.1f) {
		// 有明显水平速度，按速度方向优先
		primaryDir = (cell.m_velocityX > 0) ? 1 : -1;
		secondaryDir = -primaryDir;
	}
	else {
		// 使用确定性选择（水倾向于找到最低点）
		int deterministicChoice = rand() % 2 == 0 ? 1 : -1;
		primaryDir = deterministicChoice;
		secondaryDir = -deterministicChoice;
	}
}

int SandboxMap::GetWaterFlowDirection(const Cell& cell, int x, int y)
{
	if (std::abs(cell.m_velocityX) > 0.5f) {
		return (cell.m_velocityX > 0) ? 1 : -1;
	}

	// 检查左右哪边更"低"（更多空间）
	int leftSpaces = CountEmptySpacesBelow(x - 1, y);
	int rightSpaces = CountEmptySpacesBelow(x + 1, y);

	if (leftSpaces > rightSpaces) {
		return -1; // 向左流
	}
	else if (rightSpaces > leftSpaces) {
		return 1; // 向右流
	}
	else {
		// 空间相等，使用确定性随机
		return rand()%2==0 ? 1 : -1;
	}
}

int SandboxMap::CountEmptySpacesBelow(int x, int y)
{
	int count = 0;
	for (int dy = 1; dy <= 3; dy++) { // 检查下方3格
		if (IsInBounds(x, y - dy) && m_grid[y - dy][x].IsEmpty()) {
			count++;
		}
		else {
			break;
		}
	}
	return count;
}

void SandboxMap::ApplyWaterCollisionPhysics(Cell& cell, int deltaX, int deltaY)
{
	// 水的碰撞比沙子更有弹性
	if (deltaX != 0) {
		cell.m_velocityX *=-0.1f; // 水平反弹更强
		cell.m_velocityY *= 0.9f;
	}

	if (deltaY != 0) {
		// 垂直碰撞：更多转换为水平流动
		float absY = std::abs(cell.m_velocityY);
		float transferMomentum = absY * 0.9f; // 比沙子转换更多

		if (std::abs(cell.m_velocityX) > 0.1f) {
			cell.m_velocityX = (cell.m_velocityX > 0) ? transferMomentum : -transferMomentum;
		}
		else {
			int randomDir = rand()%2==0 ? 1 : -1;
			cell.m_velocityX = randomDir * transferMomentum;
		}

		cell.m_velocityY *= 0.6f; // 垂直速度大幅减少
	}

	// 水的摩擦更小
	cell.m_velocityX *= 0.95f;
	cell.m_velocityY *= 0.95f;
}

void SandboxMap::ApplyWaterStuckPhysics(Cell& cell)
{
	// 水被困时的物理效果（比沙子温和）
	cell.m_velocityY *= 0.9f;
	//cell.m_velocityX *= 0.8f;
}

// float SandboxMap::GetWaterRandom(int x1, int y1, int x2, int y2)
// {
// 	// 为水生成确定性随机数
// 	int hash = (x1 * 83 + y1 * 47 + x2 * 19 + y2 * 11 + m_currentFrame * 29) % 10000;
// 	return static_cast<float>(hash) / 10000.0f;
// }

bool SandboxMap::CanMoveTo(int x, int y)
{
	if (!IsInBounds(x, y)) {
		return false;
	}

	Cell& targetCell = m_grid[y][x];
	return targetCell.IsEmpty() || targetCell.m_type == CellMatType::MAT_WATER;
}

void SandboxMap::ApplySlidingPhysics(Cell& cell, int slideDirection)
{
	// 沙子斜坡滑动的物理效果
	const float FRICTION_COEFFICIENT = 0.5f;
	const float MOMENTUM_TRANSFER = 0.1f;

	// 应用摩擦力
	cell.m_velocityY *= 0.9f; // 斜向移动时垂直速度略减
	cell.m_velocityX *= FRICTION_COEFFICIENT; // 摩擦阻力

	// 垂直动量向水平动量的转换
	float momentumTransfer = std::abs(cell.m_velocityY) * MOMENTUM_TRANSFER;
	cell.m_velocityX += momentumTransfer * slideDirection;
	//cell.m_velocityX = momentumTransfer * slideDirection;

	// 限制水平速度防止过度加速
	cell.m_velocityX = GetClamped(cell.m_velocityX, -TERMINAL_SPEED * 0.5f, TERMINAL_SPEED * 0.5f);
}

void SandboxMap::ApplyHorizontalFriction(Cell& cell, int horizontalDirection)
{
	const float HORIZONTAL_FRICTION = 0.8f;          // 水平摩擦系数
	const float MOMENTUM_TRANSFER_RATE = 0.6f;        // 垂直到水平的动量转换率
	const float MIN_VELOCITY_THRESHOLD = 0.1f;        // 最小速度阈值

	// === 动量转换：垂直速度 → 水平速度 ===
	if (std::abs(cell.m_velocityX) < 0.1f) {
		 //将部分垂直动量转为水平动量（使用传入的方向）
		float transferredMomentum = std::abs(cell.m_velocityY) * MOMENTUM_TRANSFER_RATE;
		cell.m_velocityX += transferredMomentum * horizontalDirection;

		 //垂直速度因为转换而减少
		cell.m_velocityY *= (1.0f - MOMENTUM_TRANSFER_RATE);
	}

	// === 应用水平摩擦力 ===
	cell.m_velocityX *= HORIZONTAL_FRICTION;
	cell.m_velocityY *= HORIZONTAL_FRICTION;

	// === 速度阈值处理 ===
	if (std::abs(cell.m_velocityX) < MIN_VELOCITY_THRESHOLD) {
		cell.m_isFreeFalling = false;
		cell.m_velocityY = 0.f;
		cell.m_velocityY = 0.f;
		cell.m_accumulMoveY = 0.f;
		cell.m_accumulMoveX = 0.f;
	}
	if (std::abs(cell.m_velocityY) < MIN_VELOCITY_THRESHOLD) {
		cell.m_velocityY = 0.0f;
	}
}

bool SandboxMap::CanMoveHorizontally(int x, int y, const Cell& cell)
{
	return (std::abs(cell.m_velocityX) > 1.0f || std::abs(cell.m_velocityY) > 80.0f) &&
		HasSupport(x, y); // 需要有支撑才能水平移动
}

bool SandboxMap::HasSupport(int x, int y)
{
	// 在底边界就算有支撑
	if (y == 0) {
		return true;
	}

	// 检查下方是否有非空格子
	return !m_grid[y - 1][x].IsEmpty();
}

bool SandboxMap::ActOnNeighboringElement(int targetX, int targetY, bool isFinal, bool isFirst, IntVec2& currentLocation, IntVec2& lastValidLocation, int depth)
{
	// 边界检查
	if (!IsInBounds(targetX, targetY)) 
	{
		targetX = GetClamped(targetX, 0, m_mapSize.x - 1);
		targetY = GetClamped(targetY, 0, m_mapSize.y - 1);
		
		if (targetX == currentLocation.x && targetY == currentLocation.y) {
			return true;
		}
	}

	Cell& neighbor = m_grid[targetY][targetX];
	Cell& currentCell = m_grid[currentLocation.y][currentLocation.x];

	// === 情况1: 空格子（自由移动）===
	if (neighbor.IsEmpty()) {
		if (isFinal) {
			// 最终位置，执行移动
			std::swap(m_grid[currentLocation.y][currentLocation.x], m_grid[targetY][targetX]);
			currentLocation.x = targetX;
			currentLocation.y = targetY;
			m_grid[currentLocation.y][currentLocation.x].m_isFreeFalling = true;
		}
		return false; // 继续移动
	}

	// === 情况2: 水（可穿透但有阻力）===
	else if (neighbor.m_type == CellMatType::MAT_WATER) {
		if (depth > 0) {
			// 递归调用：直接穿透
			std::swap(m_grid[currentLocation.y][currentLocation.x], m_grid[targetY][targetX]);
			currentLocation.x = targetX;
			currentLocation.y = targetY;

			Cell& movedCell = m_grid[targetY][targetX];
			movedCell.m_velocityY *= 0.8f;
			movedCell.m_velocityX *= 0.8f;
			movedCell.m_isFreeFalling = true;
			return false; // 继续递归移动
		}
		else {
			// 主路径：移动并根据是否为终点决定是否停止
			std::swap(m_grid[currentLocation.y][currentLocation.x], m_grid[targetY][targetX]);
			currentLocation.x = targetX;
			currentLocation.y = targetY;

			Cell& movedCell = m_grid[targetY][targetX];
			movedCell.m_velocityY *= 0.8f;
			movedCell.m_velocityX *= 0.8f;
			movedCell.m_isFreeFalling = true;

			return isFinal; // 如果是终点就停止，否则继续
		}
	}

	// === 情况3: 固体（复杂碰撞处理，模仿Java逻辑）===
	else {
		// 防止深层递归
		if (depth > 0) return true;

		if (isFinal) {
			// 最终位置被阻挡，停在最后有效位置
			return true;
		}

		// === 物理响应：速度转换（模仿Java的逻辑）===
		if (currentCell.m_isFreeFalling) {
			float absY = std::abs(currentCell.m_velocityY);
			float transferMomentum = std::max(absY * 0.1f, 1.0f);
			transferMomentum = std::min(transferMomentum, 5.0f);

			if (std::abs(currentCell.m_velocityX) > 0.1f) {
				// 有明显水平速度，按原方向
				currentCell.m_velocityX = (currentCell.m_velocityX < 0) ? -transferMomentum : transferMomentum;
			}
			else {
				// 无明显水平速度，随机选择方向
				int randomDir = ((currentLocation.x + currentLocation.y) % 2 == 0) ? 1 : -1;
				//int randomDir = 1;
				currentCell.m_velocityX = transferMomentum * randomDir;
			}
			//currentCell.m_velocityX = GetClamped(currentCell.m_velocityX, -TERMINAL_SPEED * 0.5f, TERMINAL_SPEED * 0.5f);
		}

		// 计算尝试移动的方向
		float normalizedVelX = (currentCell.m_velocityX > 0.1f) ? 1.0f :
			(currentCell.m_velocityX < -0.1f) ? -1.0f : 0.0f;
		float normalizedVelY = (currentCell.m_velocityY > 0.1f) ? 1.0f :
			(currentCell.m_velocityY < -0.1f) ? -1.0f : 0.0f;

		int additionalX = static_cast<int>(normalizedVelX);
		int additionalY = static_cast<int>(normalizedVelY);

		// === 速度响应（模仿Java的碰撞处理）===
 		if (isFirst) {
 			// 第一次碰撞：温和处理
 			currentCell.m_velocityY *= 0.8f;
 		}
 		else {
 			// 后续碰撞：强烈反弹
 			currentCell.m_velocityY *= 0.5f;
 		}

		// 应用摩擦
		currentCell.m_velocityX *= 0.7f;

		// === 尝试1: 斜向移动 ===
     	if (additionalX != 0 && additionalY != 0) {
     		int diagonalX = currentLocation.x + additionalX;
     		int diagonalY = currentLocation.y + additionalY;
     
     		if (IsInBounds(diagonalX, diagonalY)) {
     			IntVec2 tempLocation = currentLocation;
     			bool stoppedDiagonally = ActOnNeighboringElement(
     				diagonalX, diagonalY, true, false,
     				tempLocation, lastValidLocation, depth + 1
     			);
     
     			if (!stoppedDiagonally) {
     				currentLocation = tempLocation;
    					m_grid[currentLocation.y][currentLocation.x].m_velocityX *= 0.7f;
    					m_grid[currentLocation.y][currentLocation.x].m_isFreeFalling = true;
     				return true; // 停止主路径，让斜向移动生效
     			}
     		}
     	}
 
 		// === 尝试2: 水平移动 ===
     	if (additionalX != 0) {
     		int adjacentX = currentLocation.x + additionalX;
     		int adjacentY = currentLocation.y;
     
     		if (IsInBounds(adjacentX, adjacentY) && HasSupport(adjacentX, adjacentY)) {
     			bool stoppedAdjacently = ActOnNeighboringElement(
     				adjacentX, adjacentY, true, false,
     				currentLocation, lastValidLocation, depth + 1
     			);
     
     			if (stoppedAdjacently) {
    				m_grid[currentLocation.y][currentLocation.x].m_velocityX *= -0.5f; // 水平反弹
     			}
     
     			if (!stoppedAdjacently) {
     				// 水平移动成功
    				m_grid[currentLocation.y][currentLocation.x].m_velocityX *= 0.7f; // 摩擦
    				m_grid[currentLocation.y][currentLocation.x].m_isFreeFalling = false;
     				return true;
     			}
     		}
     	}

		// === 所有尝试失败 ===
		//currentCell.m_isFreeFalling = false;
		return true; // 停止移动
	}


	return false;
}

void SandboxMap::UpdateAccumulatedMovement(int oldX, int oldY, int newX, int newY)
{
	Cell& finalCell = m_grid[newY][newX];

	int actualMoveX = newX - oldX;
	int actualMoveY = newY - oldY;

  	if (actualMoveX == 0 && actualMoveY == 0) {
  		finalCell.m_framesWithoutMovement++;
   		if (finalCell.m_framesWithoutMovement >= 80) {
   			finalCell.m_isFreeFalling = false;
   			finalCell.m_velocityY = 0.0f;
   			finalCell.m_velocityX = 0.0f;
   			finalCell.m_accumulMoveY = 0.0f;
   			finalCell.m_accumulMoveX = 0.0f;
   		}
  		return;
  	}

	float originalAccumX = finalCell.m_accumulMoveX;
	float originalAccumY = finalCell.m_accumulMoveY;

	finalCell.m_accumulMoveX -= static_cast<float>(actualMoveX);
	finalCell.m_accumulMoveY -= static_cast<float>(actualMoveY);

	// 符号保护
  	if ((originalAccumX >= 0 && finalCell.m_accumulMoveX < 0) ||
  		(originalAccumX <= 0 && finalCell.m_accumulMoveX > 0)) {
  		finalCell.m_accumulMoveX = 0.0f;
  	}
 
 	if ((originalAccumY >= 0 && finalCell.m_accumulMoveY < 0) ||
 		(originalAccumY <= 0 && finalCell.m_accumulMoveY > 0)) {
 		finalCell.m_accumulMoveY = 0.0f;
 	}
 
 	if (std::abs(finalCell.m_accumulMoveX) < 0.1f) finalCell.m_accumulMoveX = 0.0f;
 	if (std::abs(finalCell.m_accumulMoveY) < 0.1f) finalCell.m_accumulMoveY = 0.0f;
}

const char* SandboxMap::GetMaterialTypeName(CellMatType type) const
{
	switch (type)
	{
	case CellMatType::MAT_EMPTY: return "Empty";
	case CellMatType::MAT_SAND:  return "Sand";
	case CellMatType::MAT_WATER: return "Water";
	case CellMatType::MAT_STONE: return "Stone";
	default: return "Unknown";
	}
}

void SandboxMap::ApplyCollisionPhysics(Cell& cell, int deltaX, int deltaY)
{
	if (!cell.m_isFreeFalling) return;

	// 计算碰撞方向
	bool isVerticalCollision = (deltaY != 0);
	bool isHorizontalCollision = (deltaX != 0);

	if (isVerticalCollision) {
		// 垂直碰撞：速度转换为水平
		float absY = std::abs(cell.m_velocityY);
		float transferMomentum = std::max(absY * 0.8f, 1.0f);
		transferMomentum = std::min(transferMomentum, 30.0f);

		if (std::abs(cell.m_velocityX) > 0.1f) {
			// 已有水平速度，增强它
			cell.m_velocityX = (cell.m_velocityX > 0) ? transferMomentum : -transferMomentum;
		}
		else {
			// 没有水平速度，随机选择方向
			int randomDir =(rand()%2)==0?1:-1;
			cell.m_velocityX = transferMomentum * randomDir;
		}

		// 垂直速度衰减
		cell.m_velocityY *= 0.8f;
		//cell.m_velocityX *= 0.9f;
	}

	if (isHorizontalCollision) {
		// 水平碰撞：反弹
		cell.m_velocityX *= -0.5f;
		cell.m_velocityY *= 0.8f;
	}

	// 限制速度范围
	cell.m_velocityX = GetClamped(cell.m_velocityX, -TERMINAL_SPEED * 0.7f, TERMINAL_SPEED * 0.7f);
}

int SandboxMap::GetDeterministicDirection(int x, int y)
{
	// 确定性伪随机函数，避免使用rand()
	int hash = (x * 73 + y * 37 + rand()%2 * 17) % 1000;
	return (hash % 2 == 0) ? 1 : -1;
}

void SandboxMap::GetMovementDirections(const Cell& cell, int x, int y, int& primaryDir, int& secondaryDir)
{
	if (std::abs(cell.m_velocityX) > 0.2f) {
		// 有明显水平速度，按速度方向优先
		primaryDir = (cell.m_velocityX > 0) ? 1 : -1;
		secondaryDir = -primaryDir;
	}
	else {
		// 使用确定性伪随机选择
		int deterministicChoice = (rand() % 2) == 0 ? 1 : -1;
		primaryDir = deterministicChoice;
		secondaryDir = -deterministicChoice;
	}
}

void SandboxMap::ApplyHorizontalMovementPhysics(Cell& cell, int direction)
{
	// 水平移动的物理效果
	//cell.m_isFreeFalling = false; // 水平移动通常意味着有支撑

	// 应用摩擦
	cell.m_velocityX *= 0.1f;
	cell.m_velocityY *= 0.1f;
}

void SandboxMap::ApplyStuckPhysics(Cell& cell)
{
	// 无法移动时的物理效果
	cell.m_velocityY *= 0.8f; // 垂直碰撞减速
	cell.m_velocityX *= 0.7f; // 水平摩擦

	// 增加静止帧数
	cell.m_framesWithoutMovement++;

	// 如果长时间静止，停止自由落体
// 	if (cell.m_framesWithoutMovement >= 180) {
// 		cell.m_isFreeFalling = false;
// 		cell.m_velocityY = 0.0f;
// 		cell.m_velocityX = 0.0f;
// 		cell.m_accumulMoveY = 0.0f;
// 		cell.m_accumulMoveX = 0.0f;
// 	}
}

int SandboxMap::GetHorizontalDirection(const Cell& cell, int x, int y)
{
	if (std::abs(cell.m_velocityX) > 0.5f) {
		return (cell.m_velocityX > 0) ? 1 : -1;
	}
	// 如果速度很小，使用确定性选择
	return rand()%2;
}

bool SandboxMap::HandleSandCollision(int fromX, int fromY, int toX, int toY,
	Cell& movingCell, Cell& targetCell)
{
	// 计算碰撞强度
// 	float impactForce = std::sqrt(movingCell.m_velocityX * movingCell.m_velocityX +
// 		movingCell.m_velocityY * movingCell.m_velocityY);
// 
// 	// 计算激活概率（基于碰撞强度）
// 	float activationProbability = std::min(0.7f, impactForce * 0.08f);
// 
// 	// 确定性随机判断是否激活
// 	float randomValue = GetCollisionRandom(fromX, fromY, toX, toY);
// 	if (randomValue <= activationProbability) {
// 		return false; // 不激活，直接返回
// 	}

	// === 激活邻居沙粒 ===
	if (!targetCell.m_isFreeFalling) {
		targetCell.m_isFreeFalling = true;
		targetCell.m_framesWithoutMovement = 0;
		targetCell.m_accumulMoveX = 0.0f;
		targetCell.m_accumulMoveY = 0.0f;
	}

	// === 应用碰撞物理（类似普通collision，但传递动量给邻居）===
	int deltaX = toX - fromX;
	int deltaY = toY - fromY;

	// 保存移动沙粒的原始速度
	float originalVelX = movingCell.m_velocityX;
	float originalVelY = movingCell.m_velocityY;

	if (deltaY != 0) {
		// 垂直碰撞：速度转换为水平
		float absY = std::abs(movingCell.m_velocityY);
		float transferMomentum = std::min(absY * 0.15f, 1.0f);
		transferMomentum = std::min(transferMomentum, 5.0f);

		if (std::abs(movingCell.m_velocityX) > 0.1f) {
			// 已有水平速度，增强它
			movingCell.m_velocityX = (movingCell.m_velocityX > 0) ? transferMomentum : -transferMomentum;
		}
		else {
			// 没有水平速度，随机选择方向
			int randomDir = rand()%2==0 ? 1 : -1;
			movingCell.m_velocityX = transferMomentum * randomDir;
		}

		// 垂直速度衰减
		movingCell.m_velocityY *= 0.6f;
	}

	if (deltaX != 0) {
		// 水平碰撞：反弹
		movingCell.m_velocityX *= -0.3f;
		movingCell.m_velocityY *= 0.8f;
	}

	// 应用摩擦
	movingCell.m_velocityX *= 0.9f;
	movingCell.m_velocityY *= 0.9f;

	// === 将损耗的动量传递给邻居沙粒 ===
	float lostMomentumX = originalVelX - movingCell.m_velocityX;
	float lostMomentumY = originalVelY - movingCell.m_velocityY;

	// 传递动量给邻居（适当缩放）
	float transferRatio = 0.9f; // 可调参数
	targetCell.m_velocityX += lostMomentumX * transferRatio;
	targetCell.m_velocityY += lostMomentumY * transferRatio;

	// 限制邻居速度范围
	targetCell.m_velocityX = GetClamped(targetCell.m_velocityX, -TERMINAL_SPEED * 0.7f, TERMINAL_SPEED * 0.7f);
	targetCell.m_velocityY = GetClamped(targetCell.m_velocityY, -TERMINAL_SPEED * 0.7f, TERMINAL_SPEED * 0.7f);

	return false; // 没有位置交换，正常碰撞处理
}

// === 辅助函数：传递动量 ===
void SandboxMap::TransferMomentum(Cell& fromCell, Cell& toCell, float transferRatio)
{
	float transferVelX = fromCell.m_velocityX * transferRatio;
	float transferVelY = fromCell.m_velocityY * transferRatio;

	// 给目标沙粒施加力
	toCell.m_velocityX += transferVelX;
	toCell.m_velocityY += transferVelY;

	// 减少移动沙粒的动量（动量守恒）
	fromCell.m_velocityX *= (1.0f - transferRatio);
	fromCell.m_velocityY *= (1.0f - transferRatio);

	// 限制速度范围
	toCell.m_velocityX = GetClamped(toCell.m_velocityX, -TERMINAL_SPEED, TERMINAL_SPEED);
	toCell.m_velocityY = GetClamped(toCell.m_velocityY, -TERMINAL_SPEED, TERMINAL_SPEED);
	fromCell.m_velocityX = GetClamped(fromCell.m_velocityX, -TERMINAL_SPEED, TERMINAL_SPEED);
	fromCell.m_velocityY = GetClamped(fromCell.m_velocityY, -TERMINAL_SPEED, TERMINAL_SPEED);
}

// === 辅助函数：激活沙粒 ===
void SandboxMap::ActivateSandParticle(Cell& cell, float initialForce)
{
	if (cell.m_isFreeFalling) return; // 已经是自由落体

	cell.m_isFreeFalling = true;
	cell.m_framesWithoutMovement = 0;

	// 根据力的大小给予初始速度
	float baseVelocity = std::min(initialForce * 0.3f, 3.0f);

	// 随机方向分量（带有向下的偏好）
	int randomX = rand() % 2 == 0 ? 1 : -1;
	int dirX = (randomX % 3) - 1; // -1, 0, 1

	cell.m_velocityX += dirX * baseVelocity * 0.5f;
	cell.m_velocityY += baseVelocity * 0.8f; // 主要向下

	// 清除累积移动量
	cell.m_accumulMoveX = 0.0f;
	cell.m_accumulMoveY = 0.0f;
}

// === 辅助函数：尝试推动沙粒 ===
bool SandboxMap::TryPushSandParticle(int x, int y, int pushDirX, int pushDirY, float pushForce)
{
	// 计算推动目标位置
	int pushTargetX = x + pushDirX;
	int pushTargetY = y + pushDirY;

	// 边界检查
	if (!IsInBounds(pushTargetX, pushTargetY)) {
		return false;
	}

	Cell& targetCell = m_grid[pushTargetY][pushTargetX];

	// 只能推动到空位置或水中
	if (targetCell.IsEmpty() || targetCell.m_type == CellMatType::MAT_WATER) {
		Cell& sandCell = m_grid[y][x];

		// 执行推动
		std::swap(m_grid[y][x], m_grid[pushTargetY][pushTargetX]);

		// 给被推动的沙粒施加力
		Cell& pushedCell = m_grid[pushTargetY][pushTargetX];
		pushedCell.m_velocityX += pushDirX * pushForce * 0.8f;
		pushedCell.m_velocityY += pushDirY * pushForce * 0.8f;

		// 激活被推动的沙粒
		ActivateSandParticle(pushedCell, pushForce);

		return true;
	}

	// 尝试连锁推动（递归，但限制深度）
	if (targetCell.m_type == CellMatType::MAT_SAND && pushForce > 1.0f) {
		bool chainPushSuccess = TryPushSandParticle(pushTargetX, pushTargetY,
			pushDirX, pushDirY,
			pushForce * 0.6f);
		if (chainPushSuccess) {
			// 连锁推动成功，现在可以推动当前沙粒
			return TryPushSandParticle(x, y, pushDirX, pushDirY, pushForce * 0.8f);
		}
	}

	return false;
}

// === 辅助函数：沙粒碰撞物理 ===
void SandboxMap::ApplySandCollisionPhysics(Cell& cell, int deltaX, int deltaY, float impactForce)
{
	// 基于碰撞方向应用反弹和摩擦
	if (deltaX != 0) {
		// 水平碰撞：反弹并减速
		cell.m_velocityX *= -0.4f;
		cell.m_velocityY *= 0.8f;
	}

	if (deltaY != 0) {
		// 垂直碰撞：转换为水平动量
		float absY = std::abs(cell.m_velocityY);
		float transferMomentum = absY * 0.3f;

		// 根据现有水平速度或随机选择方向
		if (std::abs(cell.m_velocityX) > 0.1f) {
			cell.m_velocityX += (cell.m_velocityX > 0) ? transferMomentum : -transferMomentum;
		}
		else {
			int randomDir = rand() % 2 == 0 ? 1 : -1;
			cell.m_velocityX += ((randomDir % 2) ? 1 : -1) * transferMomentum;
		}

		cell.m_velocityY *= 0.5f;
	}

	// 应用摩擦
	cell.m_velocityX *= 0.9f;
	cell.m_velocityY *= 0.9f;
}

float SandboxMap::GetCollisionRandom(int x1, int y1, int x2, int y2)
{
	// 基于位置和当前帧生成确定性随机数
	int hash = (x1 * 73 + y1 * 37 + x2 * 17 + y2 * 13 + rand()%2 * 23) % 10000;
	return static_cast<float>(hash) / 10000.0f;
}
