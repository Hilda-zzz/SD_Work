#include "CellBehaviorSystemInChunk.hpp"
#include "CellMatManager.hpp"
#include "SandboxMap.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "CellBehaviorSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/Game.hpp"


void CellBehaviorSystemInChunk::UpdateCell(Cell& cell, int worldX, int worldY, SandboxMap* map)
{
	const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type);

	//cell.m_updatedThisFrame = true;

	switch (matDef.m_physicsType)
	{
	case PhyType::PHY_STATIC_SOLID:
		return;

	case PhyType::PHY_MOVE_SOLID:
		UpdateMoveSolid(cell, worldX, worldY, map);
		break;

	case PhyType::PHY_LIQUID:
		UpdateLiquid(cell, worldX, worldY, map);
		break;
	}
}

void CellBehaviorSystemInChunk::HandleMoveSolidMovement(int& currentX, int& currentY, int targetX, int targetY, SandboxMap* map)
{
	int totalSteps = std::abs(targetY - currentY) + std::abs(targetX - currentX);
	int remainingSteps = totalSteps;

	auto moveCallback = [&](int pathX, int pathY) -> bool {
		if (!map->IsInBounds(pathX, pathY)) {
			return false;
		}

		Cell& currentCell = map->GetCell(currentX, currentY);
		Cell& targetCell = map->GetCell(pathX, pathY);
		const CellMatDef& targetCellMatDef = CellMatManager::GetMaterialDef(targetCell.m_type);

		// === 1. If empty ===
		if (targetCell.IsEmpty()) {

			std::swap(map->GetCell(currentX, currentY), map->GetCell(pathX, pathY));
			// track new position
			currentX = pathX;
			currentY = pathY;

			MarkChunkDirtyWithNeighbors(map, currentX, currentY);
			//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();

			remainingSteps--;
			map->GetCell(currentX, currentY).m_isFreeFalling = true;
			return true;
		}

		// === 2. Liquid, replace with Buoyancy ===
		else if (targetCellMatDef.m_physicsType == PhyType::PHY_LIQUID) {

			std::swap(map->GetCell(currentX, currentY), map->GetCell(pathX, pathY));
			currentX = pathX;
			currentY = pathY;
			MarkChunkDirtyWithNeighbors(map, currentX, currentY);
			//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();

			remainingSteps--;

			// Liquid resistance
			Cell& movedCell = map->GetCell(currentX, currentY);
			movedCell.m_velocityY *= targetCellMatDef.m_viscosity;
			movedCell.m_velocityX *= targetCellMatDef.m_viscosity;
			movedCell.m_isFreeFalling = true;
			return true;
		}

		// === 3. Move solid collision ===
		else if (targetCellMatDef.m_physicsType == PhyType::PHY_MOVE_SOLID) {
			if (HandleMSvsMSCollision(currentX, currentY, pathX, pathY, currentCell, targetCell))
			{
				MarkChunkDirtyWithNeighbors(map, currentX, currentY);
				//map->GetChunkByWorldPos(pathX, pathY)->MarkDirty();
			}
			return false;
		}

		// === 4. Static solid collision ===
		else if (targetCellMatDef.m_physicsType == PhyType::PHY_STATIC_SOLID) {
			ApplyCollisionPhysics(currentCell, pathX - currentX, pathY - currentY);
			return false;
		}

		//return false;  // #TODO: Test if needed of any difference
		else {
			DebuggerPrintf("WARNING: Unknown physics type at (%d,%d): %d\n",
				pathX, pathY, (int)targetCellMatDef.m_physicsType);
			return false;
		}
	};

	BresenhamLineExcludeStart(currentX, currentY, targetX, targetY, moveCallback);

	//============ Phase 2: Cellular Automata =================================

	int maxCellularSteps = remainingSteps;
	int cellularSteps = 0;

	if (remainingSteps > 0) 
	{
		while (cellularSteps < maxCellularSteps) 
		{
			cellularSteps++;
			bool moved = false;

			Cell curCell = map->GetCell(currentX, currentY);
			int primaryDir, secondaryDir;
			GetMovementDirections(curCell, currentX, currentY, primaryDir, secondaryDir);

			const CellMatDef& matDef = CellMatManager::GetMaterialDef(curCell.m_type);

			// 1. 垂直下落
			if (map->MSCanMoveTo(currentX, currentY - 1, matDef.m_density)) {
				std::swap(map->GetCell(currentX, currentY), map->GetCell(currentX, currentY - 1));
				currentY--;
				remainingSteps--;
				moved = true;

				MarkChunkDirtyWithNeighbors(map, currentX, currentY);
				//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();

				// 保持下落状态
				map->GetCell(currentX, currentY).m_isFreeFalling = true;
			}
			// 2. 斜向移动
			else if (map->MSCanMoveTo(currentX + primaryDir, currentY - 1, matDef.m_density)) {
				std::swap(map->GetCell(currentX, currentY), map->GetCell(currentX + primaryDir, currentY - 1));
				currentX += primaryDir;
				currentY--;
				remainingSteps--;
				moved = true;

				MarkChunkDirtyWithNeighbors(map, currentX, currentY);
				//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();

				// 应用斜向移动的物理效果
				const CellMatDef& matDef = CellMatManager::GetMaterialDef(map->GetCell(currentX, currentY).m_type);
				const auto& params = matDef.m_moveSolid;

				map->GetCell(currentX, currentY).m_velocityY *= 0.9f;  // #TODO: make sure the param
				map->GetCell(currentX, currentY).m_velocityX *= matDef.m_friction;

				float momentumTransfer = std::abs(map->GetCell(currentX, currentY).m_velocityY) * 0.1f;
				map->GetCell(currentX, currentY).m_velocityX += momentumTransfer * primaryDir;
				ClampVelocity(map->GetCell(currentX, currentY), matDef.m_terminalVelocity * 0.5f);
			}
			// 3. 水平移动
			else if (map->CanMoveHorizontally(currentX, currentY, curCell) &&
				std::abs(curCell.m_velocityX) > 2.f) {
				int horizontalDir = curCell.m_velocityX > 0.f ? 1 : -1;

				if (map->MSCanMoveTo(currentX + horizontalDir, currentY, matDef.m_density)) {
					std::swap(map->GetCell(currentX, currentY), map->GetCell(currentX + horizontalDir, currentY));
					currentX += horizontalDir;
					remainingSteps--;
					moved = true;

					MarkChunkDirtyWithNeighbors(map, currentX, currentY);
					//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();

					// 应用水平移动的物理效果
					map->GetCell(currentX, currentY).m_velocityX *= matDef.m_horizontalDamping;
					map->GetCell(currentX, currentY).m_velocityY *= matDef.m_horizontalDamping;

				}
			}
		}
	}
}

void CellBehaviorSystemInChunk::HandleLiquidMovement(int& currentX, int& currentY, int targetX, int targetY, SandboxMap* map)
{
	int totalSteps = std::abs(targetY - currentY) + std::abs(targetX - currentX);
	int remainingSteps = totalSteps;

	//============phase 1. brensenham =================================
	// bool bresenhamCompleted = true;

	auto moveCallback = [&](int pathX, int pathY) -> bool {
		if (!map->IsInBounds(pathX, pathY))
		{
			// bresenhamCompleted = false;
			return false;
		}

		Cell& targetCell = map->GetCell(pathX, pathY);
		Cell& currentCell = map->GetCell(currentX, currentY);
		const CellMatDef& targetCellMatDef = CellMatManager::GetMaterialDef(targetCell.m_type);
		const CellMatDef& curCellMatDef = CellMatManager::GetMaterialDef(currentCell.m_type);

		// == 1. if empty
		if (targetCell.IsEmpty()) {
			std::swap(map->GetCell(currentX, currentY), map-> GetCell(pathX, pathY));
			// track new position
			currentX = pathX;
			currentY = pathY;
			remainingSteps--;

			MarkChunkDirtyWithNeighbors(map, currentX, currentY);
			//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();

			// keep free falling
			map->GetCell(currentX, currentY).m_isFreeFalling = true;
			return true; // keep moving
		}

		// == 2. liquid, replace with Buoyancy  
		else if (targetCellMatDef.m_density < curCellMatDef.m_density)
		{
			std::swap(map->GetCell(currentX, currentY), map->GetCell(pathX, pathY));
			currentX = pathX;
			currentY = pathY;
			MarkChunkDirtyWithNeighbors(map, currentX, currentY);
			//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();
			remainingSteps--;

			// liquid resistance
			Cell& movedCell = map->GetCell(currentX, currentY);
			movedCell.m_velocityY *= targetCellMatDef.m_viscosity;
			movedCell.m_velocityX *= targetCellMatDef.m_viscosity;
			movedCell.m_isFreeFalling = true;
			return true; // keep moving
		}
		//else if (targetCellMatDef.m_name == curCellMatDef.m_name)
		//{
		//	return true;
		//}
		else //if(targetCellMatDef.m_physicsType!=PhyType::PHY_LIQUID)
		{
			// #TODO: make sure if there need mark dirty as well as MS
			// map->GetChunkByWorldPos(pathX, pathY)->MarkDirty();
			ApplyCollisionPhysics(currentCell, pathX - currentX, pathY - currentY);
			return false;
		}
	};

	BresenhamLineExcludeStart(currentX, currentY, targetX, targetY, moveCallback);

	//============phase 2. celluar automata =================================
	int maxCellularSteps = remainingSteps+5;
	int cellularSteps = 0;
	if (remainingSteps > 0)
	{
		while (cellularSteps < maxCellularSteps)
		{
			bool moved = false;

			// diagonal direction
			int primaryDir, secondaryDir;
			Cell curCell = map->GetCell(currentX, currentY);
			GetMovementDirections(curCell, currentX, currentY, primaryDir, secondaryDir);
			cellularSteps++;

			const CellMatDef& matDef = CellMatManager::GetMaterialDef(map->GetCell(currentX, currentY).m_type);

			// 1. 优先尝试垂直下落
			if (map->LiquidCanMoveTo(currentX, currentY - 1, matDef.m_density)) {
				std::swap(map->GetCell(currentX, currentY), map->GetCell(currentX, currentY - 1));
				currentY--;
				//remainingSteps--;
				moved = true;

				MarkChunkDirtyWithNeighbors(map, currentX, currentY);
				//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();

				// 保持下落状态
				map->GetCell(currentX, currentY).m_isFreeFalling = true;

				continue;
			}
			//else if (map->LiquidCanMoveTo(currentX + primaryDir, currentY - 1, matDef.m_density))
			//{
			//	std::swap(map->GetCell(currentX, currentY), map->GetCell(currentX + primaryDir, currentY - 1));
			//	currentX += primaryDir;
			//	currentY--;
			//	//remainingSteps--;
			//	moved = true;

			//	MarkChunkDirtyWithNeighbors(map, currentX, currentY);
			//	//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();

			//	// 应用斜向移动的物理效果
			//	const CellMatDef& matDef = CellMatManager::GetMaterialDef(map->GetCell(currentX, currentY).m_type);
			//	const auto& params = matDef.m_moveSolid;

			//	map->GetCell(currentX, currentY).m_velocityY *= 0.9f;  // #TODO: make sure the param
			//	map->GetCell(currentX, currentY).m_velocityX *= matDef.m_friction;

			//	float momentumTransfer = std::abs(map->GetCell(currentX, currentY).m_velocityY) * 0.1f;
			//	map->GetCell(currentX, currentY).m_velocityX += momentumTransfer * primaryDir;
			//	ClampVelocity(map->GetCell(currentX, currentY), matDef.m_terminalVelocity * 0.5f);

			//	continue;
			//}
			else //if (map->CanMoveHorizontally(currentX, currentY, curCell))
			{
				int horizontalDir = curCell.m_velocityX >= 0.f ? 1 : -1;
				curCell.m_velocityX = horizontalDir * std::abs(map->GetCell(currentX, currentY).m_velocityY) * 0.5f;
				if (map->LiquidCanMoveTo(currentX + horizontalDir, currentY, matDef.m_density))
				{
					std::swap(map->GetCell(currentX, currentY), map->GetCell(currentX + horizontalDir, currentY));
					currentX += horizontalDir;
					moved = true;

					MarkChunkDirtyWithNeighbors(map, currentX, currentY);
					//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();

					// 应用水平移动的物理效果
					//const CellMatDef& matDef = CellMatManager::GetMaterialDef(map->GetCell(currentX, currentY).m_type);
					map->GetCell(currentX, currentY).m_velocityX *= matDef.m_horizontalDamping;
					map->GetCell(currentX, currentY).m_velocityY *= matDef.m_verticalDamping;
					map->GetCell(currentX, currentY).m_accumulMoveY = 0.f;
				}
				else if (map->LiquidCanMoveTo(currentX - horizontalDir, currentY, matDef.m_density))
				{
					map->GetCell(currentX, currentY).m_liquidReCollideTimes++;
					if (map->GetCell(currentX, currentY).m_liquidReCollideTimes > 5)
					{
						std::swap(map->GetCell(currentX, currentY), map->GetCell(currentX - horizontalDir, currentY));
						currentX -= horizontalDir;
						moved = true;

						MarkChunkDirtyWithNeighbors(map, currentX, currentY);
						//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();

						Cell& curCell = map->GetCell(currentX, currentY);
						curCell.m_velocityX = curCell.m_velocityX * (-1.f);
						curCell.m_velocityY *= matDef.m_verticalDamping;
						curCell.m_accumulMoveX = -curCell.m_accumulMoveX;
						curCell.m_accumulMoveY = 0.f;
						curCell.m_liquidReCollideTimes = 0;
					}

				}
				else
					break;
			}
		}
	}
}

bool CellBehaviorSystemInChunk::HandleMSvsMSCollision(int fromX, int fromY, int toX, int toY, Cell& movingCell, Cell& targetCell)
{
	float ratio = Game::s_rng.RollRandomFloatZeroToOne();
	const CellMatDef& targetCellMatDef = CellMatManager::GetMaterialDef(targetCell.m_type);
	if (ratio > targetCellMatDef.m_activationThreshold && !targetCell.m_isFreeFalling)
	{
		return false; // do not activate
	}
	const CellMatDef& moveCellMatDef = CellMatManager::GetMaterialDef(movingCell.m_type);
	// === Activate ===
	if (!targetCell.m_isFreeFalling)
	{
		targetCell.m_isFreeFalling = true;
		targetCell.m_framesWithoutMovement = 0;
		targetCell.m_accumulMoveX = 0.0f;
		targetCell.m_accumulMoveY = 0.0f;
	}

	// === Collision in 2 direction===  #TODO: extract as ApplyCollisionPhysics func
	int deltaX = toX - fromX;
	int deltaY = toY - fromY;

	// original velocity of move cell
	float originalVelX = movingCell.m_velocityX;
	float originalVelY = movingCell.m_velocityY;

	ApplyCollisionPhysics(movingCell, deltaX, deltaY);

	// ======================== Transfer Momentum =========================

	float lostMomentumX = originalVelX - movingCell.m_velocityX;
	float lostMomentumY = originalVelY - movingCell.m_velocityY;

	float transferRatio = moveCellMatDef.m_momentumPreservation;
	targetCell.m_velocityX += lostMomentumX * transferRatio;
	targetCell.m_velocityY += lostMomentumY * transferRatio;

	ClampVelocity(targetCell, moveCellMatDef.m_terminalVelocity * 0.7f);

	return true; // the momentum transfer happened
	// #TODO: when activate target cell , set the chunk containing the target cell to dirty
}

void CellBehaviorSystemInChunk::ApplyGravity(Cell& cell, float deltaTime)
{
	//CellBehaviorSystem::ApplyGravity(cell, deltaTime);
	const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type);

	float gravity = GRAVITY * matDef.m_gravityMultiplier;
	cell.m_velocityY += gravity * deltaTime;
	cell.m_velocityY = std::max(cell.m_velocityY, -matDef.m_terminalVelocity);
}

void CellBehaviorSystemInChunk::ApplyCollisionPhysics(Cell& cell, int deltaX, int deltaY)
{
	// 直接复用
	//CellBehaviorSystem::ApplyCollisionPhysics(cell, deltaX, deltaY);

	const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type);

	//if (matDef.m_physicsType != PhyType::PHY_MOVE_SOLID || !cell.m_isFreeFalling) return;
	if (!cell.m_isFreeFalling) return;

	const auto& params = matDef.m_moveSolid;

	if (deltaY != 0) {
		// 垂直碰撞
		float absY = std::abs(cell.m_velocityY);
		float transferMomentum = absY * matDef.m_collisionMomentumTransfer;
		transferMomentum = GetClamped(transferMomentum, 0.0f, matDef.m_terminalVelocity * 0.5f);

		if (std::abs(cell.m_velocityX) > 0.1f) {
			cell.m_velocityX = (cell.m_velocityX > 0) ?
				cell.m_velocityX * matDef.m_momentumPreservation + transferMomentum :
				cell.m_velocityX * matDef.m_momentumPreservation - transferMomentum;
		}
		else {
			int randomDir = (rand() % 100 < matDef.m_randomDirectionChance * 100) ?
				((rand() % 2) ? 1 : -1) : 1;
			cell.m_velocityX = transferMomentum * randomDir;
		}

		cell.m_velocityY *= matDef.m_verticalDamping;
	}

	if (deltaX != 0) {
		// #TODO: seems useless??
		// 水平碰撞
		cell.m_velocityX *= -matDef.m_restitution;
		cell.m_velocityY *= matDef.m_collisionDamping;
		//cell.m_velocityX *= -1.f;
		//cell.m_velocityY *= 1.f;
	}

	// 应用空气阻力
	cell.m_velocityX *= matDef.m_airResistance;
	cell.m_velocityY *= matDef.m_airResistance;

	// 限制速度
	ClampVelocity(cell, matDef.m_terminalVelocity * 0.7f);
}

void CellBehaviorSystemInChunk::ClampVelocity(Cell& cell, float maxSpeed)
{
	// 直接复用
	//CellBehaviorSystem::ClampVelocity(cell, maxSpeed);

	cell.m_velocityX = GetClamped(cell.m_velocityX, -maxSpeed, maxSpeed);
	cell.m_velocityY = GetClamped(cell.m_velocityY, -maxSpeed, maxSpeed);
}

void CellBehaviorSystemInChunk::UpdateMoveSolid(Cell& cell, int worldX, int worldY, SandboxMap* map)
{
	const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type);

	// === Update Physics ===
	if (cell.m_isFreeFalling)
	{
		MarkChunkDirtyWithNeighbors(map, worldX, worldY);
		//map->GetChunkByWorldPos(worldX, worldY)->MarkDirty();
		if (!cell.m_updatedThisFrame)
		{
			ApplyGravity(cell, map->GetDeltaTime());

			// update accumulation distance
			cell.m_accumulMoveY += cell.m_velocityY * map->GetDeltaTime();
			cell.m_accumulMoveX += cell.m_velocityX * map->GetDeltaTime();

			cell.m_updatedThisFrame = true;
		}
		else
		{
			return;
		}
	}
	else
	{
		// update isFreeFalling
		if (map->IsInBounds(worldX, worldY - 1))
		{
			CellMatDef const& neigborMatDef = CellMatManager::GetMaterialDef(map->GetCell(worldX, worldY - 1).m_type);
			if (map->GetCell(worldX, worldY - 1).IsEmpty() ||
				neigborMatDef.m_physicsType == PhyType::PHY_LIQUID)
			{
				cell.m_isFreeFalling = true;

				MarkChunkDirtyWithNeighbors(map, worldX, worldY);
				//map->GetChunkByWorldPos(worldX, worldY)->MarkDirty();

				cell.m_framesWithoutMovement = 0;
				cell.m_accumulMoveX = 0.0f;
				cell.m_accumulMoveY = 0.0f;
			}
		}
		else
		{
			return;
		}
	}

	// clamped accumulate distance
	const float MAX_ACCUMULATION = 5.0f;
	cell.m_accumulMoveY = GetClamped(cell.m_accumulMoveY, -MAX_ACCUMULATION, MAX_ACCUMULATION);
	cell.m_accumulMoveX = GetClamped(cell.m_accumulMoveX, -MAX_ACCUMULATION, MAX_ACCUMULATION);

	// check if needed to move
	if (std::abs(cell.m_accumulMoveY) < 1.0f && std::abs(cell.m_accumulMoveX) < 1.0f) {
		return;
	}

	// calculate aim position
	int targetY = worldY + static_cast<int>(cell.m_accumulMoveY);
	int targetX = worldX + static_cast<int>(cell.m_accumulMoveX);

	// Dealing with movement
	int currentX = worldX;
	int currentY = worldY;
	HandleMoveSolidMovement(currentX, currentY, targetX, targetY, map);

	// update accumulated movement
	UpdateAccumulatedMovement(worldX, worldY, currentX, currentY, map);
}

void CellBehaviorSystemInChunk::UpdateLiquid(Cell& cell, int worldX, int worldY, SandboxMap* map)
{
	const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type);

	// === Update Physics ===
	if (cell.m_isFreeFalling)
	{
		//map->GetChunkByWorldPos(worldX, worldY)->MarkDirty();
		MarkChunkDirtyWithNeighbors(map, worldX, worldY);

		if (!cell.m_updatedThisFrame)
		{
			ApplyGravity(cell, map->GetDeltaTime());

			// update accumulation distance
			cell.m_accumulMoveY += cell.m_velocityY * map->GetDeltaTime();
			cell.m_accumulMoveX += cell.m_velocityX * map->GetDeltaTime();
		}
		else
		{
			return;
		}
	}
	else
	{
		// 检查周围八个格子，任一为空就重置为自由落体
		bool hasEmptyNeighbor = false;

		// 定义8个方向的偏移量
		int offsets[13][2] = {
			{-1, 1}, {0, -1}, {1, 1},  // 上方三个
			{-1,  0},          {1,  0},  // 左右两个  
			{-1,  -1}, {0,  1}, {1,  -1},   // 下方三个
			{0,  2},
			{0,  3},
			{0,  4},
			{0,  5},
			{0,  6},
		};

		for (int i = 0; i < 13; ++i) {
			int neighborX = worldX + offsets[i][0];
			int neighborY = worldY + offsets[i][1];
			if (map->IsInBounds(neighborX, neighborY))
			{
				Cell const& neighborCell = map->GetCell(neighborX, neighborY);
				const CellMatDef& neighborMatDef = CellMatManager::GetMaterialDef(neighborCell.m_type);
				if (neighborCell.IsEmpty() || neighborMatDef.m_density < matDef.m_density)
				{
					hasEmptyNeighbor = true;
					break; // 找到一个空格子就够了
				}
			}
			else
			{
				// 边界外也算作"空"
				hasEmptyNeighbor = true;
				break;
			}
		}

		if (hasEmptyNeighbor) {
			cell.m_isFreeFalling = true;

			MarkChunkDirtyWithNeighbors(map, worldX, worldY);
			//map->GetChunkByWorldPos(worldX, worldY)->MarkDirty();

			cell.m_framesWithoutMovement = 0;
			cell.m_accumulMoveX = 0.0f;
			cell.m_accumulMoveY = 0.0f;
		}
		else {
			return;
		}
	}

	// clamped accumulate distance
	const float MAX_ACCUMULATION = 10.0f;
	cell.m_accumulMoveY = GetClamped(cell.m_accumulMoveY, -MAX_ACCUMULATION, MAX_ACCUMULATION);
	cell.m_accumulMoveX = GetClamped(cell.m_accumulMoveX, -MAX_ACCUMULATION, MAX_ACCUMULATION);

	// calculate aim position
	int targetY = worldY + static_cast<int>(cell.m_accumulMoveY);
	int targetX = worldX + static_cast<int>(cell.m_accumulMoveX);

	// Dealing with movement
	int currentX = worldX;
	int currentY = worldY;
	HandleLiquidMovement(currentX, currentY, targetX, targetY, map);

	//# TODO: make a new func for update accumulated movement liquid or combine to use one func

	// update accumulated movement
	// UpdateAccumulatedMovementLiquid(worldX, worldY, currentX, currentY);
	UpdateAccumulatedMovementLiquid(worldX, worldY, currentX, currentY, map);
}

void CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(SandboxMap* map, int worldX, int worldY)
{
	CellChunk* chunk = map->GetChunkByWorldPos(worldX, worldY);
	if (!chunk) return;

	chunk->MarkDirty();

	IntVec2 localCoords = CellChunk::WorldToLocal(worldX, worldY);

	// 检查是否在边界上并标记相邻chunk
	// 左边界
	if (localCoords.x == 0) {
		CellChunk* leftChunk = map->GetChunkByWorldPos(worldX - 1, worldY);
		if (leftChunk) leftChunk->MarkDirty();
	}
	// 右边界
	if (localCoords.x == CHUNK_SIZE - 1) {
		CellChunk* rightChunk = map->GetChunkByWorldPos(worldX + 1, worldY);
		if (rightChunk) rightChunk->MarkDirty();
	}
	// 上边界
	if (localCoords.y == 0) {
		CellChunk* topChunk = map->GetChunkByWorldPos(worldX, worldY - 1);
		if (topChunk) topChunk->MarkDirty();
	}
	// 下边界
	if (localCoords.y == CHUNK_SIZE - 1) {
		CellChunk* bottomChunk = map->GetChunkByWorldPos(worldX, worldY + 1);
		if (bottomChunk) bottomChunk->MarkDirty();
	}

	// 如果需要处理角落的情况（对角的chunk）
	// 左上角
	if (localCoords.x == 0 && localCoords.y == 0) {
		CellChunk* topLeftChunk = map->GetChunkByWorldPos(worldX - 1, worldY - 1);
		if (topLeftChunk) topLeftChunk->MarkDirty();
	}
	// 右上角
	if (localCoords.x == CHUNK_SIZE - 1 && localCoords.y == 0) {
		CellChunk* topRightChunk = map->GetChunkByWorldPos(worldX + 1, worldY - 1);
		if (topRightChunk) topRightChunk->MarkDirty();
	}
	// 左下角
	if (localCoords.x == 0 && localCoords.y == CHUNK_SIZE - 1) {
		CellChunk* bottomLeftChunk = map->GetChunkByWorldPos(worldX - 1, worldY + 1);
		if (bottomLeftChunk) bottomLeftChunk->MarkDirty();
	}
	// 右下角
	if (localCoords.x == CHUNK_SIZE - 1 && localCoords.y == CHUNK_SIZE - 1) {
		CellChunk* bottomRightChunk = map->GetChunkByWorldPos(worldX + 1, worldY + 1);
		if (bottomRightChunk) bottomRightChunk->MarkDirty();
	}
}

//void CellBehaviorSystemInChunk::MoveCellWithinChunk(CellChunk* chunk, int fromLocalX, int fromLocalY, int toLocalX, int toLocalY)
//{
//}
//
//void CellBehaviorSystemInChunk::MoveCellAcrossChunk(SandboxMap* map, int fromWorldX, int fromWorldY, int toWorldX, int toWorldY, Cell& movingCell)
//{
//	// 获取目标chunk
//	CellChunk* targetChunk = map->GetChunkByWorldPos(toWorldX, toWorldY);
//	IntVec2 targetLocal = CellChunk::WorldToLocal(toWorldX, toWorldY);
//	IntVec2 sourceChunk = CellChunk::WorldToChunkIndex(fromWorldX, fromWorldY);
//
//	// 加入pending队列（线程安全）
//	targetChunk->QueueIncomingCell(
//		targetLocal.x,
//		targetLocal.y,
//		movingCell,
//		sourceChunk  // 用于调试
//	);
//
//	// 清空源位置
//	Cell& sourceCell = map->GetCellInChunk(fromWorldX, fromWorldY);
//	sourceCell.SetEmpty();
//
//	// 标记源chunk为dirty
//	CellChunk* sourceChunkPtr = map->GetChunkByWorldPos(fromWorldX, fromWorldY);
//	sourceChunkPtr->MarkDirty();
//}

//bool CellBehaviorSystemInChunk::TryMoveCell(SandboxMap* map, int& currentX, int& currentY, int targetX, int targetY, Cell& cell)
//{
//	// 检查边界
//	if (!map->IsInBounds(targetX, targetY)) {
//		return false;
//	}
//
//	// 检查是否同一chunk
//	IntVec2 currentChunk = CellChunk::WorldToChunkIndex(currentX, currentY);
//	IntVec2 targetChunk = CellChunk::WorldToChunkIndex(targetX, targetY);
//
//	if (currentChunk == targetChunk) {
//		// === 同chunk内：直接swap ===
//		CellChunk* chunk = map->GetChunkByWorldPos(currentX, currentY);
//		IntVec2 currentLocal = CellChunk::WorldToLocal(currentX, currentY);
//		IntVec2 targetLocal = CellChunk::WorldToLocal(targetX, targetY);
//
//		chunk->SwapLocalCells(
//			currentLocal.x, currentLocal.y,
//			targetLocal.x, targetLocal.y
//		);
//
//		// 标记chunk为dirty
//		chunk->MarkDirty();
//	}
//	else {
//		// === 跨chunk：使用pending队列 ===
//		MoveCellAcrossChunk(map, currentX, currentY, targetX, targetY, cell);
//	}
//
//	// 更新当前位置
//	currentX = targetX;
//	currentY = targetY;
//
//	return true;
//}

void CellBehaviorSystemInChunk::UpdateAccumulatedMovement(int oldX, int oldY, int newX, int newY, SandboxMap* map)
{
	// === 关键：现在cell可能已经跨chunk移动 ===
	// 所以要从新位置获取cell引用

	if (!map->IsInBounds(newX, newY)) {
		return;  // 移出边界，放弃更新
	}

	Cell& finalCell = map->GetCell(newX, newY);

	int actualMoveX = newX - oldX;
	int actualMoveY = newY - oldY;

	// === 检查是否没有移动 ===
	if (actualMoveX == 0 && actualMoveY == 0) {
		finalCell.m_framesWithoutMovement++;

		//高温粒子不能置为静态  //&&!CellMatManager::GetMaterialDef(finalCell.m_type).m_isHighTemp? 或许物理化学的mark dirty应该分开处理
		if (finalCell.m_framesWithoutMovement >= 80) {
			finalCell.m_isFreeFalling = false;
			finalCell.m_velocityY = 0.0f;
			finalCell.m_velocityX = 0.0f;
			finalCell.m_accumulMoveY = 0.0f;
			finalCell.m_accumulMoveX = 0.0f;
		}
		return;
	}

	// === 更新累积移动 ===
	float originalAccumX = finalCell.m_accumulMoveX;
	float originalAccumY = finalCell.m_accumulMoveY;

	finalCell.m_accumulMoveX -= static_cast<float>(actualMoveX);
	finalCell.m_accumulMoveY -= static_cast<float>(actualMoveY);

	// === 符号保护 ===
	if ((originalAccumX >= 0 && finalCell.m_accumulMoveX < 0) ||
		(originalAccumX <= 0 && finalCell.m_accumulMoveX > 0)) {
		finalCell.m_accumulMoveX = 0.0f;
	}

	if ((originalAccumY >= 0 && finalCell.m_accumulMoveY < 0) ||
		(originalAccumY <= 0 && finalCell.m_accumulMoveY > 0)) {
		finalCell.m_accumulMoveY = 0.0f;
	}

	// === 精度处理 ===
	if (std::abs(finalCell.m_accumulMoveX) < 0.1f) finalCell.m_accumulMoveX = 0.0f;
	if (std::abs(finalCell.m_accumulMoveY) < 0.1f) finalCell.m_accumulMoveY = 0.0f;
}

void CellBehaviorSystemInChunk::UpdateAccumulatedMovementLiquid(int oldX, int oldY, int newX, int newY, SandboxMap* map)
{
	Cell& finalCell = map->GetCell(newX, newY);

	int actualMoveX = newX - oldX;
	int actualMoveY = newY - oldY;

	if (actualMoveX == 0 && actualMoveY == 0) {
		// 检查周围8个格子是否都不为空
		bool allNeighborsNonEmpty = true;

		// 定义8个方向的偏移量
		int offsets[13][2] = {
			{-1, 1}, {0, -1}, {1, 1},  // 上方三个
			{-1,  0},          {1,  0},  // 左右两个  
			{-1,  -1}, {0,  1}, {1,  -1},   // 下方三个
			{0,  2},
			{0,  3},
			{0,  4},
			{0,  5},
			{0,  6},
		};

		for (int i = 0; i < 13; ++i) {
			int neighborX = oldX + offsets[i][0];
			int neighborY = oldY + offsets[i][1];

			if (map->IsInBounds(neighborX, neighborY))
			{
				Cell const& neighborCell = map->GetCell(neighborX, neighborY);
				const CellMatDef& neighborMatDef = CellMatManager::GetMaterialDef(neighborCell.m_type);
				const CellMatDef& curMatDef = CellMatManager::GetMaterialDef(finalCell.m_type);

				if (map->GetCell(neighborX, neighborY).IsEmpty() || neighborMatDef.m_density < curMatDef.m_density)
				{
					allNeighborsNonEmpty = false;
					break; // 找到一个空格子就足够了
				}
			}
			else {
				// 边界外视为空，所以不是所有邻居都非空
				allNeighborsNonEmpty = false;
				break;
			}
		}

		// 只有在周围都不为空的情况下才增加无移动帧数
		if (allNeighborsNonEmpty) {
			finalCell.m_framesWithoutMovement++;
			if (finalCell.m_framesWithoutMovement >= 200) {
				finalCell.m_isFreeFalling = false;
				finalCell.m_velocityY = 0.0f;
				finalCell.m_velocityX = 0.0f;
				finalCell.m_accumulMoveY = 0.0f;
				finalCell.m_accumulMoveX = 0.0f;
				finalCell.m_framesWithoutMovement = 200;
			}
		}
		else {
			// 如果周围有空格子，重置无移动帧数，保持流动状态
			finalCell.m_framesWithoutMovement = 0;
		}
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

void CellBehaviorSystemInChunk::GetMovementDirections(const Cell& cell, int x, int y, int& primaryDir, int& secondaryDir)
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
