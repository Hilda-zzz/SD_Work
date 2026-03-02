#include "CellBehaviorSystemInChunk.hpp"
#include "CellMatManager.hpp"
#include "SandboxMap.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "CellBehaviorSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/Game.hpp"
#include <ThirdParty/tracy/tracy/Tracy.hpp>
#include "CABehaviors.hpp"

void CellBehaviorSystemInChunk::UpdateCell(Cell& cell, int worldX, int worldY, BaseMap* map)
{
	const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type.load());

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
	case PhyType::PHY_CELLULAR_AUTOMATON:
		//UpdateCellularAutomaton(cell, worldX, worldY, map);
		if (matDef.m_caUpdateFunc)
			matDef.m_caUpdateFunc(cell, worldX, worldY, map);
		break;
	}
}

void CellBehaviorSystemInChunk::HandleMoveSolidMovement(int& currentX, int& currentY, int targetX, int targetY, BaseMap* map)
{
	//ZoneScoped;

	int minX = currentX, maxX = currentX;
	int minY = currentY, maxY = currentY;

	int totalSteps = std::abs(targetY - currentY) + std::abs(targetX - currentX);
	int remainingSteps = totalSteps;

	bool canSkipBoundsCheck = map->IsInBounds(currentX - 32, currentY - 32) &&
		map->IsInBounds(currentX + 32, currentY + 32);

	auto moveCallback = [&](int pathX, int pathY) -> bool {

		if (!canSkipBoundsCheck)
		{
			if (!map->IsInBounds(pathX, pathY)) {
				return false;
			}
		}

		Cell& currentCell = map->GetCell(currentX, currentY);
		Cell& targetCell = map->GetCell(pathX, pathY);
		const CellMatDef& targetCellMatDef = CellMatManager::GetMaterialDef(targetCell.m_type.load());
		const CellMatDef& curCellMatDef = CellMatManager::GetMaterialDef(currentCell.m_type.load());
		// === 1. If empty ===
		if (targetCell.IsEmpty()) 
		{
			MarkChunkDirtyWithNeighbors(map, currentX, currentY);

			std::swap(currentCell, targetCell);
			// track new position
			currentX = pathX;
			currentY = pathY;

			MarkChunkDirtyWithNeighbors(map, currentX, currentY);
			minX = std::min(minX, currentX);
			maxX = std::max(maxX, currentX);
			minY = std::min(minY, currentY);
			maxY = std::max(maxY, currentY);

			//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();

			remainingSteps--;
			//map->GetCell(currentX, currentY).m_isFreeFalling = true;
			targetCell.m_isFreeFalling=true;
			return true;
		}

		// === 2. Liquid, replace with Buoyancy ===
		else if (targetCellMatDef.m_physicsType == PhyType::PHY_LIQUID||targetCellMatDef.m_physicsType==PhyType::PHY_CELLULAR_AUTOMATON) 
		{
			MarkChunkDirtyWithNeighbors(map, currentX, currentY);
	
			if (curCellMatDef.m_density > targetCellMatDef.m_density)
			{
				std::swap(currentCell, targetCell);
				currentX = pathX;
				currentY = pathY;

				MarkChunkDirtyWithNeighbors(map, currentX, currentY);
				minX = std::min(minX, currentX);
				maxX = std::max(maxX, currentX);
				minY = std::min(minY, currentY);
				maxY = std::max(maxY, currentY);

				//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();

				remainingSteps--;

				// Liquid resistance
				Cell& movedCell = map->GetCell(currentX, currentY);
				movedCell.m_velocityY *= targetCellMatDef.m_viscosity;
				movedCell.m_velocityX *= targetCellMatDef.m_viscosity;
				movedCell.m_isFreeFalling = true;
				return true;
			}
		}

		// === 3. Move solid collision ===
		else if (targetCellMatDef.m_physicsType == PhyType::PHY_MOVE_SOLID) {
			if (HandleMSvsMSCollision(currentX, currentY, pathX, pathY, currentCell, targetCell))
			{
				MarkChunkDirtyWithNeighbors(map, currentX, currentY);
				minX = std::min(minX, currentX);
				maxX = std::max(maxX, currentX);
				minY = std::min(minY, currentY);
				maxY = std::max(maxY, currentY);

				// #Check: add back
				map->GetChunkByWorldPos(pathX, pathY)->MarkDirty();
			}
			return false;
		}

		// === 4. Static solid collision ===
		else if (targetCellMatDef.m_physicsType == PhyType::PHY_STATIC_SOLID) {
			ApplyCollisionPhysics(currentCell, pathX - currentX, pathY - currentY);
			return false;
		}

		return false;  // #TODO: Test if needed of any difference
		//else {
		//	DebuggerPrintf("WARNING: Unknown physics type at (%d,%d): %d\n",
		//		pathX, pathY, (int)targetCellMatDef.m_physicsType);
		//	return false;
		//}
	};

	BresenhamLineExcludeStart(currentX, currentY, targetX, targetY, moveCallback);

	//============ Phase 2: Cellular Automata =================================

	{
		//ZoneScopedN("CAMove");
	int maxCellularSteps = remainingSteps;
	int cellularSteps = 0;

	if (remainingSteps > 0) 
	{
		const CellMatDef& matDef = CellMatManager::GetMaterialDef(map->GetCell(currentX, currentY).m_type.load());
		while (cellularSteps < maxCellularSteps) 
		{
			cellularSteps++;
			bool moved = false;

			Cell& curCell = map->GetCell(currentX, currentY);
			int primaryDir, secondaryDir;
			GetMovementDirections(curCell, currentX, currentY, primaryDir, secondaryDir);

			//const CellMatDef& matDef = CellMatManager::GetMaterialDef(curCell.m_type.load());

			// 1. 垂直下落
			if (map->MSCanMoveTo(currentX, currentY - 1, matDef.m_density, canSkipBoundsCheck)) {
				Cell& targetCell = map->GetCell(currentX, currentY - 1);
				std::swap(curCell, targetCell);
				currentY--;
				remainingSteps--;
				moved = true;

				MarkChunkDirtyWithNeighbors(map, currentX, currentY);
				minX = std::min(minX, currentX);
				maxX = std::max(maxX, currentX);
				minY = std::min(minY, currentY);
				maxY = std::max(maxY, currentY);

				//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();

				// 保持下落状态
				//map->GetCell(currentX, currentY).m_isFreeFalling = true;
				targetCell.m_isFreeFalling = true;
			}
			// 2. 斜向移动
			else if (map->MSCanMoveTo(currentX + primaryDir, currentY - 1, matDef.m_density, canSkipBoundsCheck)) {
				Cell& targetCell = map->GetCell(currentX + primaryDir, currentY - 1);
				std::swap(curCell, targetCell);
				currentX += primaryDir;
				currentY--;
				remainingSteps--;
				moved = true;

				MarkChunkDirtyWithNeighbors(map, currentX, currentY);
				minX = std::min(minX, currentX);
				maxX = std::max(maxX, currentX);
				minY = std::min(minY, currentY);
				maxY = std::max(maxY, currentY);

				//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();

				// 应用斜向移动的物理效果
				const CellMatDef& matDef = CellMatManager::GetMaterialDef(targetCell.m_type.load());
				const auto& params = matDef.m_moveSolid;

				targetCell.m_velocityY *= 0.9f;  // #TODO: make sure the param
				targetCell.m_velocityX *= matDef.m_friction;

				float momentumTransfer = std::abs(targetCell.m_velocityY) * 0.1f;
				targetCell.m_velocityX += momentumTransfer * primaryDir;
				ClampVelocity(targetCell, matDef.m_terminalVelocity * 0.5f);
			}
			// 3. 水平移动
			else if (map->CanMoveHorizontally(currentX, currentY, curCell) &&
				std::abs(curCell.m_velocityX) > 2.f) {
				int horizontalDir = curCell.m_velocityX > 0.f ? 1 : -1;

				if (map->MSCanMoveTo(currentX + horizontalDir, currentY, matDef.m_density, canSkipBoundsCheck)) {
					Cell& targetCell = map->GetCell(currentX + horizontalDir, currentY);
					std::swap(curCell, targetCell);
					currentX += horizontalDir;
					remainingSteps--;
					moved = true;

					MarkChunkDirtyWithNeighbors(map, currentX, currentY);
					minX = std::min(minX, currentX);
					maxX = std::max(maxX, currentX);
					minY = std::min(minY, currentY);
					maxY = std::max(maxY, currentY);

					//map->GetChunkByWorldPos(currentX, currentY)->MarkDirty();

					// 应用水平移动的物理效果
					targetCell.m_velocityX *= matDef.m_horizontalDamping;
					targetCell.m_velocityY *= matDef.m_horizontalDamping;

				}
			}
		}
	}
	}

	//MarkChunkDirtyInRange(map, minX, minY, maxX, maxY);
}

void CellBehaviorSystemInChunk::HandleLiquidMovement(int& currentX, int& currentY, int targetX, int targetY, BaseMap* map)
{
	int totalSteps = std::abs(targetY - currentY) + std::abs(targetX - currentX);
	int remainingSteps = totalSteps;

	bool canSkipBoundsCheck = map->IsInBounds(currentX - 32, currentY - 32) &&
		map->IsInBounds(currentX + 32, currentY + 32);

	//============phase 1. brensenham =================================
	// bool bresenhamCompleted = true;

	auto moveCallback = [&](int pathX, int pathY) -> bool {
		if (!canSkipBoundsCheck)
		{
			if (!map->IsInBounds(pathX, pathY))
			{
				// bresenhamCompleted = false;
				return false;
			}
		}

		Cell& targetCell = map->GetCell(pathX, pathY);
		Cell& currentCell = map->GetCell(currentX, currentY);
		const CellMatDef& targetCellMatDef = CellMatManager::GetMaterialDef(targetCell.m_type.load());
		const CellMatDef& curCellMatDef = CellMatManager::GetMaterialDef(currentCell.m_type.load());

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

			const CellMatDef& matDef = CellMatManager::GetMaterialDef(map->GetCell(currentX, currentY).m_type.load());

			// 1. 优先尝试垂直下落
			if (map->LiquidCanMoveTo(currentX, currentY - 1, matDef.m_density, canSkipBoundsCheck)) {
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
				if (map->LiquidCanMoveTo(currentX + horizontalDir, currentY, matDef.m_density, canSkipBoundsCheck))
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
				else if (map->LiquidCanMoveTo(currentX - horizontalDir, currentY, matDef.m_density, canSkipBoundsCheck))
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
	const CellMatDef& targetCellMatDef = CellMatManager::GetMaterialDef(targetCell.m_type.load());
	if (ratio > targetCellMatDef.m_activationThreshold && !targetCell.m_isFreeFalling)
	{
		return false; // do not activate
	}
	const CellMatDef& moveCellMatDef = CellMatManager::GetMaterialDef(movingCell.m_type.load());
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
	const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type.load());

	float gravity = GRAVITY * matDef.m_gravityMultiplier;
	cell.m_velocityY += gravity * deltaTime;
	cell.m_velocityY = std::max(cell.m_velocityY, -matDef.m_terminalVelocity);
}

void CellBehaviorSystemInChunk::ApplyCollisionPhysics(Cell& cell, int deltaX, int deltaY)
{
	// 直接复用
	//CellBehaviorSystem::ApplyCollisionPhysics(cell, deltaX, deltaY);

	const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type.load());

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

void CellBehaviorSystemInChunk::UpdateMoveSolid(Cell& cell, int worldX, int worldY, BaseMap* map)
{
	//ZoneScoped;

	// === Update Physics ===
	if (cell.m_isFreeFalling)
	{
		MarkChunkDirtyWithNeighbors(map, worldX, worldY);
		//map->GetChunkByWorldPos(worldX, worldY)->MarkDirty();
		if (!cell.m_updatedThisFrame)
		{
			float deltaTime = map->GetDeltaTime();
			ApplyGravity(cell, deltaTime);

			// update accumulation distance
			cell.m_accumulMoveY += cell.m_velocityY * deltaTime;
			cell.m_accumulMoveX += cell.m_velocityX * deltaTime;

			cell.m_updatedThisFrame = true;
		}
		else
		{
			return;
		}
	}
	else
	{
		//// update isFreeFalling
		//if (map->IsInBounds(worldX, worldY - 1))
		//{
		//	Cell const& belowNeighborCell = map->GetCell(worldX, worldY - 1);
		//	CellMatDef const& neigborMatDef = CellMatManager::GetMaterialDef(belowNeighborCell.m_type.load());
		//	if (belowNeighborCell.IsEmpty() ||
		//		neigborMatDef.m_physicsType == PhyType::PHY_LIQUID)
		//	{
		//		cell.m_isFreeFalling = true;

		//		MarkChunkDirtyWithNeighbors(map, worldX, worldY);
		//		//map->GetChunkByWorldPos(worldX, worldY)->MarkDirty();

		//		cell.m_framesWithoutMovement = 0;
		//		cell.m_accumulMoveX = 0.0f;
		//		cell.m_accumulMoveY = 0.0f;
		//	}
		//}
		//else
		//{
		//	return;
		//}
		if (map->IsInBounds(worldX, worldY - 1))
		{
			Cell const& belowCell = map->GetCell(worldX, worldY - 1);
			CellMatDef const& belowMatDef = CellMatManager::GetMaterialDef(belowCell.m_type.load());

			bool belowPassable = belowCell.IsEmpty() || belowMatDef.m_physicsType == PhyType::PHY_LIQUID;

			bool diagPassable = false;

			bool leftDiagPassable = false;
			bool rightDiagPassable = false;

			// 检查左下
			if (map->IsInBounds(worldX - 1, worldY - 1))
			{
				Cell const& diagLeft = map->GetCell(worldX - 1, worldY - 1);
				CellMatDef const& diagLeftMatDef = CellMatManager::GetMaterialDef(diagLeft.m_type.load());
				if (diagLeft.IsEmpty() || diagLeftMatDef.m_physicsType == PhyType::PHY_LIQUID)
					leftDiagPassable = true;
			}

			// 检查右下
			if (map->IsInBounds(worldX + 1, worldY - 1))
			{
				Cell const& diagRight = map->GetCell(worldX + 1, worldY - 1);
				CellMatDef const& diagRightMatDef = CellMatManager::GetMaterialDef(diagRight.m_type.load());
				if (diagRight.IsEmpty() || diagRightMatDef.m_physicsType == PhyType::PHY_LIQUID)
					rightDiagPassable = true;
			}

			// 左下和右下都满足才激活
			diagPassable = leftDiagPassable && rightDiagPassable;

			if (belowPassable || diagPassable)
			{
				cell.m_isFreeFalling = true;
				MarkChunkDirtyWithNeighbors(map, worldX, worldY);
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
	if (currentX != targetX || currentY != targetY)
		HandleMoveSolidMovement(currentX, currentY, targetX, targetY, map);

	// update accumulated movement
	UpdateAccumulatedMovement(worldX, worldY, currentX, currentY, map);
}

void CellBehaviorSystemInChunk::UpdateLiquid(Cell& cell, int worldX, int worldY, BaseMap* map)
{
	const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type.load());

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
				const CellMatDef& neighborMatDef = CellMatManager::GetMaterialDef(neighborCell.m_type.load());
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

void CellBehaviorSystemInChunk::UpdateCellularAutomaton(Cell& cell, int worldX, int worldY, BaseMap* map)
{
	//const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type);

	//if (cell.m_updatedThisFrame) {
	//	return;
	//}

	//cell.m_updatedThisFrame = true;

	//// === 检查是否处于活动状态 ===
	//if (!cell.m_isFreeFalling)
	//{
	//	bool shouldActivate = false;

	//	// 检查正下方
	//	if (map->IsInBounds(worldX, worldY - 1))
	//	{
	//		Cell& belowCell = map->GetCell(worldX, worldY - 1);
	//		const CellMatDef& belowMatDef = CellMatManager::GetMaterialDef(belowCell.m_type);

	//		// 正下方为空或液体，激活
	//		if (belowCell.IsEmpty() || belowMatDef.m_physicsType == PhyType::PHY_LIQUID)
	//		{
	//			shouldActivate = true;
	//		}
	//		else
	//		{
	//			// 正下方被固体占据，检查左下和右下
	//			// 检查左下
	//			if (map->IsInBounds(worldX - 1, worldY - 1))
	//			{
	//				Cell& leftDiagCell = map->GetCell(worldX - 1, worldY - 1);
	//				if (leftDiagCell.IsEmpty())
	//				{
	//					shouldActivate = true;
	//				}
	//			}

	//			// 检查右下
	//			if (!shouldActivate && map->IsInBounds(worldX + 1, worldY - 1))
	//			{
	//				Cell& rightDiagCell = map->GetCell(worldX + 1, worldY - 1);
	//				if (rightDiagCell.IsEmpty())
	//				{
	//					shouldActivate = true;
	//				}
	//			}
	//		}
	//	}

	//	if (shouldActivate)
	//	{
	//		cell.m_isFreeFalling = true;
	//		cell.m_framesWithoutMovement = 0;
	//	}
	//	else
	//	{
	//		cell.m_framesWithoutMovement++;
	//		return;  // 仍然静止，跳过更新
	//	}
	//}

	//MarkChunkDirtyWithNeighbors(map, worldX, worldY);

	//int newX = worldX;
	//int newY = worldY;
	//bool moved = false;

	//// === 元胞自动机移动规则 ===
	//if (map->IsInBounds(worldX, worldY - 1)) {
	//	Cell& belowCell = map->GetCell(worldX, worldY - 1);

	//	if (belowCell.IsEmpty()) {
	//		std::swap(map->GetCell(worldX, worldY), map->GetCell(worldX, worldY - 1));
	//		newY = worldY - 1;
	//		moved = true;
	//		MarkChunkDirtyWithNeighbors(map, newX, newY);
	//		map->GetCell(newX, newY).m_framesWithoutMovement = 0;
	//		return;
	//	}

	//	const CellMatDef& belowMatDef = CellMatManager::GetMaterialDef(belowCell.m_type);

	//	if (belowMatDef.m_physicsType == PhyType::PHY_LIQUID) {
	//		std::swap(map->GetCell(worldX, worldY), map->GetCell(worldX, worldY - 1));
	//		newY = worldY - 1;
	//		moved = true;
	//		MarkChunkDirtyWithNeighbors(map, newX, newY);
	//		map->GetCell(newX, newY).m_framesWithoutMovement = 0;
	//		return;
	//	}

	//	int leftDiag = worldX - 1;
	//	int rightDiag = worldX + 1;
	//	int diagY = worldY - 1;

	//	bool canMoveLeft = false;
	//	bool canMoveRight = false;

	//	if (map->IsInBounds(leftDiag, diagY)) {
	//		Cell& leftDiagCell = map->GetCell(leftDiag, diagY);
	//		if (leftDiagCell.IsEmpty()) {
	//			canMoveLeft = true;
	//		}
	//		else {
	//			const CellMatDef& leftDiagMatDef = CellMatManager::GetMaterialDef(leftDiagCell.m_type);
	//			if (leftDiagMatDef.m_physicsType == PhyType::PHY_LIQUID) {
	//				canMoveLeft = true;
	//			}
	//		}
	//	}

	//	if (map->IsInBounds(rightDiag, diagY)) {
	//		Cell& rightDiagCell = map->GetCell(rightDiag, diagY);
	//		if (rightDiagCell.IsEmpty()) {
	//			canMoveRight = true;
	//		}
	//		else {
	//			const CellMatDef& rightDiagMatDef = CellMatManager::GetMaterialDef(rightDiagCell.m_type);
	//			if (rightDiagMatDef.m_physicsType == PhyType::PHY_LIQUID) {
	//				canMoveRight = true;
	//			}
	//		}
	//	}

	//	int moveDir = 0;

	//	if (canMoveLeft && canMoveRight) {
	//		moveDir = (rand() % 2 == 0) ? -1 : 1;
	//	}
	//	else if (canMoveLeft) {
	//		moveDir = -1;
	//	}
	//	else if (canMoveRight) {
	//		moveDir = 1;
	//	}

	//	if (moveDir != 0) {
	//		newX = worldX + moveDir;
	//		newY = diagY;
	//		std::swap(map->GetCell(worldX, worldY), map->GetCell(newX, newY));
	//		moved = true;
	//		MarkChunkDirtyWithNeighbors(map, newX, newY);
	//		map->GetCell(newX, newY).m_framesWithoutMovement = 0;
	//		return;
	//	}
	//}

	//// === 无法移动：累计静止帧数 ===
	//if (!moved) 
	//{
	//	Cell& finalCell = map->GetCell(worldX, worldY);
	//	finalCell.m_framesWithoutMovement++;

	//	if (finalCell.m_framesWithoutMovement >= 80) 
	//	{
	//		finalCell.m_isFreeFalling = false;
	//	}
	//}

//-----------------------------------------------------------------------------------------------
	// ExtraData状态: 0 = 未初始化, 1 = Falling, 2 = Stationary

	// ExtraData状态: 1 = Falling, 2 = Stationary

	bool moved = false;
	MarkChunkDirtyWithNeighbors(map, worldX, worldY);
	// ========================================================================
	// 规则1: 根据下方状态设置 extra data
	// ========================================================================
	if (map->IsInBounds(worldX, worldY - 1))
	{
		Cell& belowCell = map->GetCell(worldX, worldY - 1);

		if (belowCell.IsEmpty())
		{
			cell.m_extraData = 1;  // Falling
		}
		else
		{
			cell.m_extraData = 2;  // Stationary
		}
	}

	// ========================================================================
	// 规则2: extra data == 1 (Falling)
	// ========================================================================
	if (cell.m_extraData == 1)
	{
		// 执行两次
		for (int attempt = 0; attempt < 3; attempt++)
		{
			// 50%概率触发
			if (rand() % 3 == 0)
			{
				// 随机选择下落方向：0=左下, 1=正下, 2=右下
				int direction = rand() % 3;
				int targetX = worldX;
				int targetY = worldY - 1;

				if (direction == 0) {
					targetX = worldX - 1;  // 左下
				}
				else if (direction == 2) {
					targetX = worldX + 1;  // 右下
				}
				// direction == 1 时保持 targetX = worldX (正下)

				if (map->IsInBounds(targetX, targetY))
				{
					Cell& targetCell = map->GetCell(targetX, targetY);
					if (targetCell.IsEmpty())
					{
						std::swap(map->GetCell(worldX, worldY), map->GetCell(targetX, targetY));
						worldX = targetX;
						worldY = targetY;
						moved = true;
						MarkChunkDirtyWithNeighbors(map, worldX, worldY);
					}
				}
			}
		}
	}
	//	//if (moved) return;
	//}

	// ========================================================================
	// 规则3: extra data == 2 (Stationary)
	// ========================================================================
	if (cell.m_extraData == 2)
	{
		// 3.1 执行一次：水平漂移
		{
			// 随机一个水平方向
			int randomDir = (rand() % 2 == 0) ? -1 : 1;
			int leftX = worldX - 1;
			int rightX = worldX + 1;
			int targetX = worldX + randomDir;

			if (map->IsInBounds(leftX, worldY) &&
				map->IsInBounds(rightX, worldY) &&
				map->IsInBounds(worldX, worldY - 1))
			{
				Cell& leftCell = map->GetCell(leftX, worldY);
				Cell& rightCell = map->GetCell(rightX, worldY);
				Cell& belowCell = map->GetCell(worldX, worldY - 1);
				const CellMatDef& belowMatDef = CellMatManager::GetMaterialDef(belowCell.m_type.load());

				// 条件：左右都为空 && 下方不是static solid && 下方不为空
				if (leftCell.IsEmpty() &&
					rightCell.IsEmpty()) //&&
					//!belowCell.IsEmpty() )
					//belowMatDef.m_physicsType != PhyType::PHY_STATIC_SOLID)
				{
					// 与随机方向交换
					if (map->IsInBounds(targetX, worldY))
					{
						std::swap(map->GetCell(worldX, worldY), map->GetCell(targetX, worldY));
						moved = true;
						MarkChunkDirtyWithNeighbors(map, targetX, worldY);
						//return;
					}
				}
			}
		}

		// 3.2 随机一个水平方向，执行两次：斜上方有雪时下落
		{
			int randomDir = (rand() % 2 == 0) ? -1 : 1;
			int diagUpX = worldX + randomDir;
			int diagUpY = worldY + 1;  // 斜上方

			for (int attempt = 0; attempt < 2; attempt++)
			{
				if (map->IsInBounds(worldX, worldY - 1) &&
					map->IsInBounds(diagUpX, diagUpY))
				{
					Cell& belowCell = map->GetCell(worldX, worldY - 1);
					Cell& diagUpCell = map->GetCell(diagUpX, diagUpY);

					// 如果下方为空 并且 斜上方为snow
					if (belowCell.IsEmpty() && diagUpCell.m_type.load() == CellMatType::MAT_CA_SNOW)
					{
						// 与下方交换
						std::swap(map->GetCell(worldX, worldY), map->GetCell(worldX, worldY - 1));
						worldY = worldY - 1;
						moved = true;
						MarkChunkDirtyWithNeighbors(map, worldX, worldY);
						// 继续执行第二次
					}
					else
					{
						break;
					}
				}
			}

			//if (moved) return;
		}
	}

	//// ========================================================================
	//// 规则4: 如果下方是液体，与之交换；执行两次
	//// ========================================================================
	//for (int attempt = 0; attempt < 2; attempt++)
	//{
	//	if (map->IsInBounds(worldX, worldY - 1))
	//	{
	//		Cell& belowCell = map->GetCell(worldX, worldY - 1);
	//		const CellMatDef& belowMatDef = CellMatManager::GetMaterialDef(belowCell.m_type);

	//		if (belowMatDef.m_physicsType == PhyType::PHY_LIQUID)
	//		{
	//			std::swap(map->GetCell(worldX, worldY), map->GetCell(worldX, worldY - 1));
	//			worldY = worldY - 1;
	//			moved = true;
	//			MarkChunkDirtyWithNeighbors(map, worldX, worldY);
	//			// 继续执行第二次
	//		}
	//		else
	//		{
	//			break;
	//		}
	//	}
	//}

	////if (moved) return;

	// ========================================================================
	// 规则5: 如果 extra data == 1，执行五次：被风吹起
	// ========================================================================
	if (cell.m_extraData == 1)
	{
		for (int attempt = 0; attempt < 4; attempt++)
		{
			// 随机一个水平方向
			int randomDir = (rand() % 2 == 0) ? -1 : 1;
			int diagUpX = worldX + randomDir;
			int diagUpY = worldY + 1;  // 斜上方

			// 统计周围八个位置的空格子数量
			int emptyCount = 0;
			for (int dy = -1; dy <= 1; dy++)
			{
				for (int dx = -1; dx <= 1; dx++)
				{
					if (dx == 0 && dy == 0) continue;
					int checkX = worldX + dx;
					int checkY = worldY + dy;
					if (map->IsInBounds(checkX, checkY))
					{
						Cell& checkCell = map->GetCell(checkX, checkY);
						if (checkCell.IsEmpty())
						{
							emptyCount++;
						}
					}
				}
			}

			// 条件：周围有大于6个empty
			if (emptyCount > 6)
			{
				if (map->IsInBounds(diagUpX, diagUpY))
				{
					Cell& diagUpCell = map->GetCell(diagUpX, diagUpY);

					// 条件：斜上方为空
					if (diagUpCell.IsEmpty())
					{
						// 检查斜上方是否与snow相邻
						bool touchingSnow = false;
						for (int dy = -1; dy <= 1; dy++)
						{
							for (int dx = -1; dx <= 1; dx++)
							{
								if (dx == 0 && dy == 0) continue;
								int checkX = diagUpX + dx;
								int checkY = diagUpY + dy;
								if (map->IsInBounds(checkX, checkY))
								{
									Cell& checkCell = map->GetCell(checkX, checkY);
									if (checkCell.m_type.load() == CellMatType::MAT_CA_SNOW)
									{
										touchingSnow = true;
										break;
									}
								}
							}
							if (touchingSnow) break;
						}

						std::swap(map->GetCell(worldX, worldY), map->GetCell(diagUpX, diagUpY));
						moved = true;
						MarkChunkDirtyWithNeighbors(map, diagUpX, diagUpY);

						// 条件：不与snow相邻
						if (!touchingSnow)
						{
							// 与斜上方交换
							
							//return;
						}
					}
				}
			}
		}
	}

	// ========================================================================
	// 规则6: 如果左、右、上都是snow，下方为空，与上方交换
	// ========================================================================
	if (map->IsInBounds(worldX - 1, worldY) &&
		map->IsInBounds(worldX + 1, worldY) &&
		map->IsInBounds(worldX, worldY + 1) &&
		map->IsInBounds(worldX, worldY - 1))
	{
		Cell& leftCell = map->GetCell(worldX - 1, worldY);
		Cell& rightCell = map->GetCell(worldX + 1, worldY);
		Cell& upCell = map->GetCell(worldX, worldY + 1);
		Cell& downCell = map->GetCell(worldX, worldY - 1);

		if (leftCell.m_type.load() == CellMatType::MAT_CA_SNOW &&
			rightCell.m_type.load() == CellMatType::MAT_CA_SNOW &&
			upCell.m_type.load() == CellMatType::MAT_CA_SNOW &&
			downCell.IsEmpty())
		{
			// 与上方交换
			std::swap(map->GetCell(worldX, worldY), map->GetCell(worldX, worldY + 1));
			moved = true;
			MarkChunkDirtyWithNeighbors(map, worldX, worldY + 1);
			//return;
		}
	}
}

void CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(BaseMap* map, int worldX, int worldY)
{
	//ZoneScoped;

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

void CellBehaviorSystemInChunk::MarkChunkDirtyInRange(BaseMap* map, int minX, int minY, int maxX, int maxY)
{
	//ZoneScoped;
	// 快速路径:单点标记
	if (minX == maxX && minY == maxY) {
		MarkChunkDirtyWithNeighbors(map, minX, minY);
		return;
	}

	// 获取局部坐标
	IntVec2 minLocal = CellChunk::WorldToLocal(minX, minY);
	IntVec2 maxLocal = CellChunk::WorldToLocal(maxX, maxY);

	// === 核心思想:最多9个chunk,用数组存储去重 ===
	CellChunk* chunks[9] = { nullptr };
	int chunkCount = 0;

	auto addChunkIfUnique = [&](CellChunk* chunk) {
		if (!chunk) return;

		// 检查是否已存在
		for (int i = 0; i < chunkCount; i++) {
			if (chunks[i] == chunk) return;
		}

		// 添加新chunk
		chunks[chunkCount++] = chunk;
		};

	// 标记四个顶点所在的chunk
	addChunkIfUnique(map->GetChunkByWorldPos(minX, minY));
	addChunkIfUnique(map->GetChunkByWorldPos(maxX, minY));
	addChunkIfUnique(map->GetChunkByWorldPos(minX, maxY));
	addChunkIfUnique(map->GetChunkByWorldPos(maxX, maxY));

	// 检查边界并添加相邻chunk

	// 左边界
	if (minLocal.x == 0) {
		addChunkIfUnique(map->GetChunkByWorldPos(minX - 1, minY));
	}

	// 右边界
	if (maxLocal.x == CHUNK_SIZE - 1) {
		addChunkIfUnique(map->GetChunkByWorldPos(maxX + 1, minY));
	}

	// 下边界
	if (minLocal.y == 0) {
		addChunkIfUnique(map->GetChunkByWorldPos(minX, minY - 1));
	}

	// 上边界
	if (maxLocal.y == CHUNK_SIZE - 1) {
		addChunkIfUnique(map->GetChunkByWorldPos(minX, maxY + 1));
	}

	// 检查四个角
	if (minLocal.x == 0 && minLocal.y == 0) {
		addChunkIfUnique(map->GetChunkByWorldPos(minX - 1, minY - 1));
	}
	if (maxLocal.x == CHUNK_SIZE - 1 && minLocal.y == 0) {
		addChunkIfUnique(map->GetChunkByWorldPos(maxX + 1, minY - 1));
	}
	if (minLocal.x == 0 && maxLocal.y == CHUNK_SIZE - 1) {
		addChunkIfUnique(map->GetChunkByWorldPos(minX - 1, maxY + 1));
	}
	if (maxLocal.x == CHUNK_SIZE - 1 && maxLocal.y == CHUNK_SIZE - 1) {
		addChunkIfUnique(map->GetChunkByWorldPos(maxX + 1, maxY + 1));
	}

	// 批量标记所有chunk
	for (int i = 0; i < chunkCount; i++) {
		chunks[i]->MarkDirty();
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

void CellBehaviorSystemInChunk::UpdateAccumulatedMovement(int oldX, int oldY, int newX, int newY, BaseMap* map)
{
	// === 关键：现在cell可能已经跨chunk移动 ===
	// 所以要从新位置获取cell引用

	// #RiskyDelete
	//if (!map->IsInBounds(newX, newY)) {
	//	return;  // 移出边界，放弃更新
	//}

	Cell& finalCell = map->GetCell(newX, newY);

	int actualMoveX = newX - oldX;
	int actualMoveY = newY - oldY;

	// === 检查是否没有移动 ===
	if (actualMoveX == 0 && actualMoveY == 0) 
	{
		finalCell.m_framesWithoutMovement++;

		//高温粒子不能置为静态  //&&!CellMatManager::GetMaterialDef(finalCell.m_type).m_isHighTemp? 或许物理化学的mark dirty应该分开处理
		if (finalCell.m_framesWithoutMovement >= 80) 
		{
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

void CellBehaviorSystemInChunk::UpdateAccumulatedMovementLiquid(int oldX, int oldY, int newX, int newY, BaseMap* map)
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

		const CellMatDef& curMatDef = CellMatManager::GetMaterialDef(finalCell.m_type.load());
		for (int i = 0; i < 13; ++i) {
			int neighborX = oldX + offsets[i][0];
			int neighborY = oldY + offsets[i][1];

			if (map->IsInBounds(neighborX, neighborY))
			{
				Cell const& neighborCell = map->GetCell(neighborX, neighborY);
				const CellMatDef& neighborMatDef = CellMatManager::GetMaterialDef(neighborCell.m_type.load());
			
				if (neighborCell.IsEmpty() || neighborMatDef.m_density < curMatDef.m_density)
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
