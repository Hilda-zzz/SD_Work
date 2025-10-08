#include "CellBehaviorSystem.hpp"
#include "CellMatManager.hpp"
#include "SandboxMap.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cmath>
#include <map>
#include "Game.hpp"

void CellBehaviorSystem::UpdateCell(Cell& cell, int x, int y, SandboxMap* map)
{
	const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type);

	switch (matDef.m_physicsType)
	{
	case PhyType::PHY_STATIC_SOLID:
		return;

	case PhyType::PHY_MOVE_SOLID:
		UpdateMoveSolid(cell, x, y, map);
		break;

	case PhyType::PHY_LIQUID:
		UpdateLiquid(cell, x, y, map);
		break;
	}
}

void CellBehaviorSystem::HandleMoveSolideMovement(int& currentX, int& currentY, int targetX, int targetY, SandboxMap* map)
{
	int totalSteps = std::abs(targetY - currentY) + std::abs(targetX - currentX);
	int remainingSteps = totalSteps;

	//============phase 1. brensenham =================================
	bool bresenhamCompleted = true;

	auto moveCallback = [&](int pathX, int pathY) -> bool {
		if (!map->IsInBounds(pathX, pathY))
		{
			bresenhamCompleted = false;
			return false;
		}

		Cell& targetCell = map->GetCell(pathX, pathY);
		Cell& currentCell = map->GetCell(currentX, currentY);
		const CellMatDef& targetCellMatDef = CellMatManager::GetMaterialDef(targetCell.m_type);

		// == 1. if empty
		if (targetCell.IsEmpty()) {
			std::swap(map->GetCell(currentX, currentY), map->GetCell(pathX, pathY));
			// track new position
			currentX = pathX;
			currentY = pathY;
			remainingSteps--;

			// keep free falling
			map->GetCell(currentX, currentY).m_isFreeFalling = true;
			return true; // keep moving
		}

		// == 2. liquid, replace with Buoyancy  
		// #TODO: replace by compare density
		else if (targetCellMatDef.m_physicsType == PhyType::PHY_LIQUID) {
			std::swap(map->GetCell(currentX, currentY), map->GetCell(pathX, pathY));
			currentX = pathX;
			currentY = pathY;
			remainingSteps--;

			// liquid resistance
			Cell& movedCell = map->GetCell(currentX, currentY);
			movedCell.m_velocityY *= targetCellMatDef.m_viscosity;
			movedCell.m_velocityX *= targetCellMatDef.m_viscosity;
			movedCell.m_isFreeFalling = true;
			return true; // keep moving
		}

		// == 3. move solid
		else if (targetCellMatDef.m_physicsType == PhyType::PHY_MOVE_SOLID) {

			bool collisionHandled = HandleMSvsMSCollision(
				currentX, currentY,
				pathX, pathY,
				currentCell, targetCell
			);

			return false; //#TODO: check this return 
		}

		// == 4. static solid
		else if (targetCellMatDef.m_physicsType == PhyType::PHY_STATIC_SOLID){
			ApplyCollisionPhysics(currentCell, pathX - currentX, pathY - currentY);
			return false;
		}
	};

	BresenhamLineExcludeStart(currentX, currentY, targetX, targetY, moveCallback);

	//============phase 2. celluar automata =================================
	int maxCellularSteps = remainingSteps;
	int cellularSteps = 0;
	if (remainingSteps > 0)
	{
		while (cellularSteps < maxCellularSteps)
		{
			cellularSteps++;
			bool moved = false;

			// diagonal direction
			int primaryDir, secondaryDir;
			Cell curCell = map->GetCell(currentX, currentY);
			map->GetMovementDirections(curCell, currentX, currentY, primaryDir, secondaryDir);
			const CellMatDef& matDef = CellMatManager::GetMaterialDef(map->GetCell(currentX, currentY).m_type);
			// 1. 优先尝试垂直下落
			if (map->MSCanMoveTo(currentX, currentY - 1, matDef.m_density)) {
				std::swap(map->GetCell(currentX,currentY), map->GetCell(currentX, currentY-1));
				currentY--;
				remainingSteps--;
				moved = true;

				// 保持下落状态
				map->GetCell(currentX, currentY).m_isFreeFalling = true;
			}
			else if (map->MSCanMoveTo(currentX + primaryDir, currentY - 1, matDef.m_density))
			{
				std::swap(map->GetCell(currentX, currentY), map->GetCell(currentX + primaryDir, currentY-1));
				currentX += primaryDir;
				currentY--;
				remainingSteps--;
				moved = true;

				// 应用斜向移动的物理效果
				const CellMatDef& matDef = CellMatManager::GetMaterialDef(map->GetCell(currentX, currentY).m_type);
				const auto& params = matDef.m_moveSolid;

				map->GetCell(currentX, currentY).m_velocityY *= 0.9f;  // #TODO: make sure the param
				map->GetCell(currentX, currentY).m_velocityX *= matDef.m_friction;

				float momentumTransfer = std::abs(map->GetCell(currentX, currentY).m_velocityY) * 0.1f;
				map->GetCell(currentX, currentY).m_velocityX += momentumTransfer * primaryDir;
				ClampVelocity(map->GetCell(currentX, currentY), matDef.m_terminalVelocity * 0.5f);
			}
			else if (map->CanMoveHorizontally(currentX, currentY, curCell) && abs(curCell.m_velocityX) > 2.f) {
				int horizontalDir = curCell.m_velocityX > 0.f ? 1 : -1;

				if (map->MSCanMoveTo(currentX + horizontalDir, currentY, matDef.m_density))
				{
					std::swap(map->GetCell(currentX, currentY), map->GetCell(currentX + horizontalDir, currentY));
					currentX += horizontalDir;
					remainingSteps--;
					moved = true;

					// 应用水平移动的物理效果
					
					map->GetCell(currentX, currentY).m_velocityX *= matDef.m_horizontalDamping;
					map->GetCell(currentX, currentY).m_velocityY *= matDef.m_horizontalDamping;
				}
			}
		}
	}
}

bool CellBehaviorSystem::HandleMSvsMSCollision(int fromX, int fromY, int toX, int toY,
	Cell& movingCell, Cell& targetCell)
{	
	float ratio = Game::s_rng.RollRandomFloatZeroToOne();
	const CellMatDef& targetCellMatDef = CellMatManager::GetMaterialDef(targetCell.m_type);
	if (ratio > targetCellMatDef.m_activationThreshold&&!targetCell.m_isFreeFalling)
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
}

void CellBehaviorSystem::HandleLiquidMovement(int& currentX, int& currentY, int targetX, int targetY, SandboxMap* map)
{
	int totalSteps = std::abs(targetY - currentY) + std::abs(targetX - currentX);
	int remainingSteps = totalSteps;

	//============phase 1. brensenham =================================
	bool bresenhamCompleted = true;

	auto moveCallback = [&](int pathX, int pathY) -> bool {
		if (!map->IsInBounds(pathX, pathY))
		{
			bresenhamCompleted = false;
			return false;
		}

		Cell& targetCell = map->GetCell(pathX, pathY);
		Cell& currentCell = map->GetCell(currentX, currentY);
		const CellMatDef& targetCellMatDef = CellMatManager::GetMaterialDef(targetCell.m_type);
		const CellMatDef& curCellMatDef = CellMatManager::GetMaterialDef(currentCell.m_type);

		// == 1. if empty
		if (targetCell.IsEmpty()) {
			std::swap(map->GetCell(currentX, currentY), map->GetCell(pathX, pathY));
			// track new position
			currentX = pathX;
			currentY = pathY;
			remainingSteps--;

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
			remainingSteps--;

			// liquid resistance
			Cell& movedCell = map->GetCell(currentX, currentY);
			movedCell.m_velocityY *= targetCellMatDef.m_viscosity;
			movedCell.m_velocityX *= targetCellMatDef.m_viscosity;
			movedCell.m_isFreeFalling = true;
			return true; // keep moving
		}
		else //if(targetCellMatDef.m_physicsType!=PhyType::PHY_LIQUID)
		{
			ApplyCollisionPhysics(currentCell, pathX - currentX, pathY - currentY);
			return false;
		}
	};

	BresenhamLineExcludeStart(currentX, currentY, targetX, targetY, moveCallback);

	//============phase 2. celluar automata =================================
	int maxCellularSteps = remainingSteps;
	int cellularSteps = 0;
	if (remainingSteps > 0)
	{
		while (cellularSteps < maxCellularSteps)
		{
			bool moved = false;

			// diagonal direction
			int primaryDir, secondaryDir;
			Cell curCell = map->GetCell(currentX, currentY);
			map->GetMovementDirections(curCell, currentX, currentY, primaryDir, secondaryDir);
			cellularSteps++;

			const CellMatDef& matDef = CellMatManager::GetMaterialDef(map->GetCell(currentX, currentY).m_type);

			// 1. 优先尝试垂直下落
			if (map->LiquidCanMoveTo(currentX, currentY - 1, matDef.m_density)) {
				std::swap(map->GetCell(currentX, currentY), map->GetCell(currentX, currentY - 1));
				currentY--;
				//remainingSteps--;
				moved = true;

				// 保持下落状态
				map->GetCell(currentX, currentY).m_isFreeFalling = true;
			}
			else if (map->LiquidCanMoveTo(currentX + primaryDir, currentY - 1, matDef.m_density))
			{
				std::swap(map->GetCell(currentX, currentY), map->GetCell(currentX + primaryDir, currentY - 1));
				currentX += primaryDir;
				currentY--;
				//remainingSteps--;
				moved = true;

				// 应用斜向移动的物理效果
				const CellMatDef& matDef = CellMatManager::GetMaterialDef(map->GetCell(currentX, currentY).m_type);
				const auto& params = matDef.m_moveSolid;

				map->GetCell(currentX, currentY).m_velocityY *= 0.9f;  // #TODO: make sure the param
				map->GetCell(currentX, currentY).m_velocityX *= matDef.m_friction;

				float momentumTransfer = std::abs(map->GetCell(currentX, currentY).m_velocityY) * 0.1f;
				map->GetCell(currentX, currentY).m_velocityX += momentumTransfer * primaryDir;
				ClampVelocity(map->GetCell(currentX, currentY), matDef.m_terminalVelocity * 0.5f);
			}
			else //if (map->CanMoveHorizontally(currentX, currentY, curCell))
			{
				int horizontalDir = curCell.m_velocityX >= 0.f ? 1 : -1;
				curCell.m_velocityX = horizontalDir*std::abs(map->GetCell(currentX, currentY).m_velocityY) * 0.5f;
				if (map->LiquidCanMoveTo(currentX + horizontalDir, currentY, matDef.m_density))
				{
					std::swap(map->GetCell(currentX, currentY), map->GetCell(currentX + horizontalDir, currentY));
					currentX += horizontalDir;
					moved = true;


					// 应用水平移动的物理效果
					//const CellMatDef& matDef = CellMatManager::GetMaterialDef(map->GetCell(currentX, currentY).m_type);
					map->GetCell(currentX, currentY).m_velocityX *= matDef.m_horizontalDamping;
					map->GetCell(currentX, currentY).m_velocityY *= matDef.m_verticalDamping;
					map->GetCell(currentX, currentY).m_accumulMoveY = 0.f;
				}
				else if(map->LiquidCanMoveTo(currentX - horizontalDir, currentY, matDef.m_density))
				{
					map->GetCell(currentX, currentY).m_liquidReCollideTimes++;
					if (map->GetCell(currentX, currentY).m_liquidReCollideTimes > 5)
					{
						std::swap(map->GetCell(currentX, currentY), map->GetCell(currentX - horizontalDir, currentY));
						currentX -= horizontalDir;
						moved = true;
						
						map->GetCell(currentX, currentY).m_velocityX = curCell.m_velocityX * (-1.f);
						map->GetCell(currentX, currentY).m_velocityY *= matDef.m_verticalDamping;
						map->GetCell(currentX, currentY).m_accumulMoveX = -curCell.m_accumulMoveX;
						map->GetCell(currentX, currentY).m_accumulMoveY = 0.f;
						map->GetCell(currentX, currentY).m_liquidReCollideTimes = 0;
					}

				}
			}
		}
	}
}

void CellBehaviorSystem::UpdateMoveSolid(Cell& cell, int x, int y, SandboxMap* map)
{
	const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type);

	// === Update Physics ===
	if (cell.m_isFreeFalling) 
	{
		ApplyGravity(cell, map->GetDeltaTime());

		// update accumulation distance
		cell.m_accumulMoveY += cell.m_velocityY * map->GetDeltaTime();
		cell.m_accumulMoveX += cell.m_velocityX * map->GetDeltaTime();
	}
	else 
	{
		// update isFreeFalling
		if (map->IsInBounds(x, y - 1)) 
		{
			CellMatDef const& neigborMatDef = CellMatManager::GetMaterialDef(map->GetCell(x, y - 1).m_type);
			if (map->GetCell(x, y - 1).IsEmpty() ||
				neigborMatDef.m_physicsType == PhyType::PHY_LIQUID)
			{
				cell.m_isFreeFalling = true;
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
	int targetY = y + static_cast<int>(cell.m_accumulMoveY);
	int targetX = x + static_cast<int>(cell.m_accumulMoveX);

	// Dealing with movement
	int currentX = x;
	int currentY = y;
	HandleMoveSolideMovement(currentX, currentY, targetX, targetY, map);

	// update accumulated movement
	map->UpdateAccumulatedMovement(x, y, currentX, currentY);
}

void CellBehaviorSystem::UpdateLiquid(Cell& cell, int x, int y, SandboxMap* map)
{
	const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type);

	// color variation
	//if (cell.m_velocityX > 0.1f)
	//	cell.m_color = Rgba8::CYAN;
	//else if (cell.m_velocityX < -0.1f)
	//	cell.m_color = Rgba8(30, 100, 255);
	//else
	//	cell.m_color = Rgba8(30, 144, 255);

	// === Update Physics ===
	if (cell.m_isFreeFalling)
	{
		ApplyGravity(cell, map->GetDeltaTime());

		// update accumulation distance
		cell.m_accumulMoveY += cell.m_velocityY * map->GetDeltaTime();
		cell.m_accumulMoveX += cell.m_velocityX * map->GetDeltaTime();
	}
	else
	{
		// 检查周围八个格子，任一为空就重置为自由落体
		bool hasEmptyNeighbor = false;
		
		// 定义8个方向的偏移量
		int offsets[8][2] = {
			{-1, 1}, {0, -1}, {1, 1},  // 上方三个
			{-1,  0},          {1,  0},  // 左右两个  
			{-1,  -1}, {0,  1}, {1,  -1}   // 下方三个
		};

		for (int i = 0; i < 8; ++i) {
			int neighborX = x + offsets[i][0];
			int neighborY = y + offsets[i][1];
			if (map->IsInBounds(neighborX, neighborY)) 
			{
				Cell const& neighborCell = map->GetCell(neighborX, neighborY);
				const CellMatDef& neighborMatDef = CellMatManager::GetMaterialDef(neighborCell.m_type);
				if (neighborCell.IsEmpty()|| neighborMatDef.m_density<matDef.m_density) 
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
			cell.m_framesWithoutMovement = 0;
			cell.m_accumulMoveX = 0.0f;
			cell.m_accumulMoveY = 0.0f;
		}
		else {
			return;
		}
	}

	// clamped accumulate distance
	const float MAX_ACCUMULATION = 5.0f;
	cell.m_accumulMoveY = GetClamped(cell.m_accumulMoveY, -MAX_ACCUMULATION, MAX_ACCUMULATION);
	cell.m_accumulMoveX = GetClamped(cell.m_accumulMoveX, -MAX_ACCUMULATION, MAX_ACCUMULATION);

	// calculate aim position
	int targetY = y + static_cast<int>(cell.m_accumulMoveY);
	int targetX = x + static_cast<int>(cell.m_accumulMoveX);

	// Dealing with movement
	int currentX = x;
	int currentY = y;
	HandleLiquidMovement(currentX, currentY, targetX, targetY, map);

	// update accumulated movement
	map->UpdateAccumulatedMovementLiquid(x, y, currentX, currentY);
}

void CellBehaviorSystem::ClampVelocity(Cell& cell, float maxSpeed)
{
	cell.m_velocityX = GetClamped(cell.m_velocityX, -maxSpeed, maxSpeed);
	cell.m_velocityY = GetClamped(cell.m_velocityY, -maxSpeed, maxSpeed);
}

void CellBehaviorSystem::ApplyGravity(Cell& cell, float deltaTime)
{
	const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type);

	float gravity = GRAVITY * matDef.m_gravityMultiplier;
	cell.m_velocityY += gravity * deltaTime;
	cell.m_velocityY = std::max(cell.m_velocityY, -matDef.m_terminalVelocity);
}

void CellBehaviorSystem::ApplyCollisionPhysics(Cell& cell, int deltaX, int deltaY)
{
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
		// 水平碰撞
		cell.m_velocityX *= -matDef.m_restitution;
		cell.m_velocityY *= matDef.m_collisionDamping;
	}

	// 应用空气阻力
	cell.m_velocityX *= matDef.m_airResistance;
	cell.m_velocityY *= matDef.m_airResistance;

	// 限制速度
	ClampVelocity(cell, matDef.m_terminalVelocity * 0.7f);
}

void CellBehaviorSystem::ApplyLiquidCollisionPhysics(Cell& cell, int deltaX, int deltaY)
{

}
