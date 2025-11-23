#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Game/World.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Chunk.hpp"

extern Renderer* g_theRenderer;

Entity::Entity(Game* owner)
	: m_game(owner)
	, m_position(Vec3(0.f, 0.f, 0.f))
	, m_velocity(Vec3(0.f, 0.f, 0.f))
	, m_orientation(EulerAngles(0.f, 0.f, 0.f))
	, m_angularVelocity(EulerAngles(0.f, 0.f, 0.f))
	, m_acceleration(Vec3(0.f, 0.f, 0.f))
	, m_verticalVelocity(0.f)
	, m_isOnGround(false)
	, m_eyeHeight(1.65f)
{
	//Vec3 center = Vec3(0.f,0.f,0.f);
	Vec3 halfExtents(g_playerWidth / 2.0f, g_playerWidth / 2.0f, g_playerHeight / 2.0f);
	m_physicsAABB.m_mins =  - halfExtents;
	m_physicsAABB.m_maxs =  halfExtents;
	
	AddVertsForAABB3D(m_wireFrameVerts, m_physicsAABB, m_color);
}

Entity::~Entity()
{
	m_game = nullptr;
}

void Entity::Render() const
{
	g_theRenderer->SetModelConstants(GetModelToWorldTransform_WithoutOrientation());
	g_theRenderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_NONE);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->DrawVertexArray(m_wireFrameVerts);

	//DebugAddWorldPoint(m_position, 0.5f, 0.f);
}

Mat44 Entity::GetModelToWorldTransform() const
{
	Mat44 modelToWorld = Mat44::MakeTranslation3D(m_position);
	modelToWorld.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
	return modelToWorld;
}

Mat44 Entity::GetModelToWorldTransform_WithoutOrientation() const
{
	Mat44 modelToWorld = Mat44::MakeTranslation3D(m_position);
	return modelToWorld;
}

void Entity::UpdatePhysics(float deltaSeconds)
{
	// 1. 速度积分：v += a * dt
	m_velocity += m_acceleration * deltaSeconds;

	// 2. 限制最大速度
	Vec2 horizontalVel(m_velocity.x, m_velocity.y);
	if (horizontalVel.GetLength() > g_playerMaxHorizontalSpeed)
	{
		horizontalVel.SetLength(g_playerMaxHorizontalSpeed);
		m_velocity.x = horizontalVel.x;
		m_velocity.y = horizontalVel.y;
	}
	m_velocity.z = GetClamped(m_velocity.z, -g_playerMaxVerticalSpeed, g_playerMaxVerticalSpeed);

	// 3. 计算期望位移
	Vec3 deltaPosition = m_velocity * deltaSeconds;

	// 4. 碰撞检测与解决（如果不是NOCLIP）
	if (m_physicsMode != PhysicsMode::NOCLIP)
	{
		Vec3 resolvedDelta = ResolveCollisionsWithRaycasts(deltaPosition);

		// ✅ 根据实际移动的距离来更新速度（碰撞后速度修正）
		if (deltaSeconds > 0.0f)
		{
			// 如果某个轴的移动被完全阻止，清零该轴的速度
			if (fabsf(resolvedDelta.x) < fabsf(deltaPosition.x) * 0.001f)
			{
				m_velocity.x = 0.f;
			}
			if (fabsf(resolvedDelta.y) < fabsf(deltaPosition.y) * 0.001f)
			{
				m_velocity.y = 0.f;
			}
			if (fabsf(resolvedDelta.z) < fabsf(deltaPosition.z) * 0.001f)
			{
				m_velocity.z = 0.f;
			}
		}

		deltaPosition = resolvedDelta;
	}
	
	// 5. 应用最终位移
	m_position += deltaPosition;

	//if (m_physicsMode != PhysicsMode::NOCLIP) {
	//	//DepenetrateSolidBlocks();
	//}

	CheckGroundCollision();

	// 6. ⚠️ 重置加速度（下一帧重新累积）
	m_acceleration = Vec3(0.f, 0.f, 0.f);
}

//Vec3 Entity::ResolveCollisionsWithRaycasts(const Vec3& deltaPosition)
//{
//	// 获取12个角点
//	std::vector<Vec3> corners = GetCornerPositions();
//
//	// 射线方向和距离
//	Vec3 direction = deltaPosition.GetNormalized();
//	float distance = deltaPosition.GetLength() + 0.01f;
//
//	// 记录每个轴的最近碰撞
//	AxisImpact xImpact, yImpact, zImpact;
//
//	// 对每个角点执行射线检测
//	for (const Vec3& corner : corners)
//	{
//		Vec3 startPos = GetShrunkCorner(corner) + m_position;
//		DebugAddWorldPoint(startPos, 0.05f, 0.f);
//		RaycastResult3D result = m_world->RaycastVsBlocks(startPos, direction, distance);
//
//		if (result.m_didImpact)
//		{
//			DebugAddWorldPoint(result.m_impactPos, 0.1f, 0.f,Rgba8::RED);
//			// 过滤背面碰撞
//			//float dot = DotProduct3D(result.m_impactNormal, direction);
//			//if (dot >= 0.f) continue;
//
//			// 分类到对应的轴
//			Vec3 absNormal(fabs(result.m_impactNormal.x), fabs(result.m_impactNormal.y), fabs(result.m_impactNormal.z));
//			float impactFraction = result.m_impactDist / result.m_rayMaxLength;
//			if (absNormal.x > absNormal.y && absNormal.x > absNormal.z)
//			{
//				if (impactFraction < xImpact.m_impactFraction)
//				{
//					xImpact.m_hasImpact = true;
//					xImpact.m_impactFraction = impactFraction;
//					xImpact.m_normal = result.m_impactNormal;
//				}
//			}
//			else if (absNormal.y > absNormal.z)
//			{
//				if (impactFraction < yImpact.m_impactFraction)
//				{
//					yImpact.m_hasImpact = true;
//					yImpact.m_impactFraction = impactFraction;
//					yImpact.m_normal = result.m_impactNormal;
//				}
//			}
//			else
//			{
//				if (impactFraction < zImpact.m_impactFraction)
//				{
//					zImpact.m_hasImpact = true;
//					zImpact.m_impactFraction = impactFraction;
//					zImpact.m_normal = result.m_impactNormal;
//				}
//			}
//		}
//	}
//
//	Vec3 resolvedDelta = deltaPosition;
//
//	if (xImpact.m_hasImpact)
//	{
//		// ✅ 移动到碰撞点，而不是完全不移动
//		resolvedDelta.x *= xImpact.m_impactFraction;
//		m_velocity.x = 0.f;
//	}
//
//	if (yImpact.m_hasImpact)
//	{
//		// ✅ 移动到碰撞点
//		resolvedDelta.y *= yImpact.m_impactFraction;
//		m_velocity.y = 0.f;
//	}
//
//	if (zImpact.m_hasImpact)
//	{
//		// ✅ 移动到碰撞点（地面）
//		resolvedDelta.z *= zImpact.m_impactFraction;
//		m_velocity.z = 0.f;
//	}
//
//	return resolvedDelta;
//}

Vec3 Entity::ResolveCollisionsWithRaycasts(const Vec3& deltaPosition)
{
	// Get 12 corner positions
	std::vector<Vec3> corners = GetCornerPositions();

	// Ray direction and distance
	Vec3 direction = deltaPosition.GetNormalized();
	float distance = deltaPosition.GetLength() + 0.01f;

	// Track the minimum impact fraction for each axis
	float minXImpactFraction = 1.0f;
	float minYImpactFraction = 1.0f;
	float minZImpactFraction = 1.0f;

	bool hasXImpact = false;
	bool hasYImpact = false;
	bool hasZImpact = false;

	// Perform raycast for each corner
	for (const Vec3& corner : corners)
	{
		Vec3 startPos = GetShrunkCorner(corner) + m_position;
		//DebugAddWorldPoint(startPos, 0.05f, 0.f);

		RaycastResult3D result = m_world->RaycastVsBlocks(startPos, direction, distance);

		if (result.m_didImpact)
		{
			//DebugAddWorldPoint(result.m_impactPos, 0.1f, 0.f, Rgba8::RED);

			// ✅ Ignore back-facing collisions (moving away from the wall)
			float dot = DotProduct3D(result.m_impactNormal, direction);
			if (dot >= 0.f) continue;

			// Calculate impact fraction
			float impactFraction = result.m_impactDist / distance;

			// Get absolute normal values to determine which axis was hit
			Vec3 absNormal(fabsf(result.m_impactNormal.x),
				fabsf(result.m_impactNormal.y),
				fabsf(result.m_impactNormal.z));

			// Classify impact by axis (based on which component of normal is largest)
			if (absNormal.x > absNormal.y && absNormal.x > absNormal.z)
			{
				// ✅ Check if velocity is moving towards the wall on X axis
				float velocityDotNormal = deltaPosition.x * result.m_impactNormal.x;
				if (velocityDotNormal < 0.f)  // Moving into the wall
				{
					if (impactFraction < minXImpactFraction)
					{
						minXImpactFraction = impactFraction;
						hasXImpact = true;
					}
				}
			}
			else if (absNormal.y > absNormal.z)
			{
				// ✅ Check if velocity is moving towards the wall on Y axis
				float velocityDotNormal = deltaPosition.y * result.m_impactNormal.y;
				if (velocityDotNormal < 0.f)  // Moving into the wall
				{
					if (impactFraction < minYImpactFraction)
					{
						minYImpactFraction = impactFraction;
						hasYImpact = true;
					}
				}
			}
			else
			{
				// ✅ Check if velocity is moving towards the wall on Z axis
				float velocityDotNormal = deltaPosition.z * result.m_impactNormal.z;
				if (velocityDotNormal < 0.f)  // Moving into the wall
				{
					if (impactFraction < minZImpactFraction)
					{
						minZImpactFraction = impactFraction;
						hasZImpact = true;
					}
				}
			}
		}
	}

	// Resolve each axis independently
	Vec3 resolvedDelta = deltaPosition;

	if (hasXImpact)
	{
		resolvedDelta.x = deltaPosition.x * minXImpactFraction;
		//m_velocity.x = 0.f;
	}

	if (hasYImpact)
	{
		resolvedDelta.y = deltaPosition.y * minYImpactFraction;
		//m_velocity.y = 0.f;
	}

	if (hasZImpact)
	{
		resolvedDelta.z = deltaPosition.z * minZImpactFraction;
		//m_velocity.z = 0.f;
	}

	return resolvedDelta;
}

void Entity::CheckGroundCollision()
{
	// 1. 如果不是WALKING模式，直接返回false
	if (m_physicsMode != PhysicsMode::WALKING)
	{
		m_isOnGround = false;
		return;
	}

	// 2. 获取entity的朝向向量（用于计算4个底部角点）
	Vec3 forward, left, up;
	m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

	// 3. 计算AABB底部中心点（世界坐标）
	float halfWidth = g_playerWidth / 2.0f;  // 0.3f
	Vec3 baseCenter = m_position - Vec3(0.f, 0.f, g_playerHeight / 2.f) + Vec3(0.0f, 0.0f, GROUND_RAYCAST_OFFSET);

	// 4. 构建4个底部角点（使用朝向向量）
	Vec3 corners[4];
	corners[0] = baseCenter + forward * halfWidth + left * halfWidth;   // 前左
	corners[1] = baseCenter + forward * halfWidth - left * halfWidth;   // 前右
	corners[2] = baseCenter - forward * halfWidth + left * halfWidth;   // 后左
	corners[3] = baseCenter - forward * halfWidth - left * halfWidth;   // 后右

	// 5. 射线参数
	Vec3 rayDirection(0.0f, 0.0f, -1.0f);  // 向下
	float rayLength = GROUND_RAY_LENGTH;    // 2 * offset = 0.02m

	// 6. 对每个角点执行射线检测
	bool foundGround = false;

	for (int i = 0; i < 4; i++)
	{
		Vec3 rayStart = corners[i];

		RaycastResult3D result = m_world->RaycastVsBlocks(rayStart, rayDirection, rayLength);

		// 7. 只有当击中面的法线主要指向上方(Z+)时才认为是地面
		if (result.m_didImpact)
		{
			// ✅ 检查法线的Z分量是否为正（指向上）
			// 并且Z分量必须是法线中占主导的分量（最大的绝对值）
			Vec3 absNormal(fabsf(result.m_impactNormal.x),
				fabsf(result.m_impactNormal.y),
				fabsf(result.m_impactNormal.z));

			// 法线必须主要指向Z轴，并且是向上的(+Z)
			if (absNormal.z > absNormal.x && absNormal.z > absNormal.y && result.m_impactNormal.z > 0.0f)
			{
				foundGround = true;
				// 找到有效地面后可以提前退出
				break;
			}
		}
	}

	// 8. 更新grounded状态
	m_isOnGround = foundGround;
}

Vec3 Entity::GetEyePosition() const
{
	return m_position+Vec3(0.f,0.f,-g_playerHeight/2.f)+Vec3(0.f,0.f,m_eyeHeight);
}

std::vector<Vec3> Entity::GetCornerPositions()
{
	std::vector<Vec3> corners;

	float minX = m_physicsAABB.m_mins.x;
	float maxX = m_physicsAABB.m_maxs.x;
	float minY = m_physicsAABB.m_mins.y;
	float maxY = m_physicsAABB.m_maxs.y;
	float minZ = m_physicsAABB.m_mins.z;
	float maxZ = m_physicsAABB.m_maxs.z;
	float midZ = (minZ + maxZ) * 0.5f;

	// 底部4个角（脚部）
	corners.push_back(Vec3(minX, minY, minZ));
	corners.push_back(Vec3(maxX, minY, minZ));
	corners.push_back(Vec3(minX, maxY, minZ));
	corners.push_back(Vec3(maxX, maxY, minZ));

	// 中部4个角（腰部）
	corners.push_back(Vec3(minX, minY, midZ));
	corners.push_back(Vec3(maxX, minY, midZ));
	corners.push_back(Vec3(minX, maxY, midZ));
	corners.push_back(Vec3(maxX, maxY, midZ));

	// 顶部4个角（头部）
	corners.push_back(Vec3(minX, minY, maxZ));
	corners.push_back(Vec3(maxX, minY, maxZ));
	corners.push_back(Vec3(minX, maxY, maxZ));
	corners.push_back(Vec3(maxX, maxY, maxZ));

	return corners;
}

Vec3 Entity::GetShrunkCorner(const Vec3& corner)
{
	// 计算本地AABB的中心（本地坐标）
	Vec3 localCenter = (m_physicsAABB.m_mins + m_physicsAABB.m_maxs) * 0.5f;
	// = ((−0.3,−0.3,−0.9) + (0.3,0.3,0.9)) * 0.5 = (0, 0, 0)

	// 从本地中心指向本地角点的方向
	Vec3 direction = corner - localCenter;

	if (direction.GetLengthSquared() > 0.0001f)
	{
		direction.Normalized();
		return corner - direction * CORNER_OFFSET;
	}

	return corner;
}

void Entity::DepenetrateSolidBlocks()
{
	const int MAX_DEPENETRATION_ITERATIONS = 4;  // 防止无限循环
	const float DEPENETRATION_EPSILON = 0.001f;   // 最小穿透阈值

	// 获取所有12个角点
	std::vector<Vec3> corners = GetCornerPositions();

	for (int iteration = 0; iteration < MAX_DEPENETRATION_ITERATIONS; iteration++)
	{
		bool foundPenetration = false;
		Vec3 totalPushOut(0.f, 0.f, 0.f);

		// 检查每个角点是否在固体方块内
		for (const Vec3& corner : corners)
		{
			Vec3 worldCorner = corner + m_position;

			// 使用BlockIterator检查该位置是否是固体方块
			IntVec3 blockCoords(
				static_cast<int>(floorf(worldCorner.x)),
				static_cast<int>(floorf(worldCorner.y)),
				static_cast<int>(floorf(worldCorner.z))
			);

			// 获取起始方块的区块和索引
			Chunk* startChunk = nullptr;
			startChunk = m_world->GetChunkByWorldPos(corner);
			if (!startChunk)
			{
				continue;
			}

			// 计算起始方块在区块内的局部坐标
			int blockIndex = startChunk->GlobalCoordsToIndex(blockCoords);

			// 创建方块迭代器
			BlockIterator blockIter(startChunk, blockIndex);

//---------------------

			// 如果角点在固体方块内，计算推出向量
			if (blockIter.IsOpaque())
			{
				foundPenetration = true;

				// 计算方块中心
				Vec3 blockCenter(
					blockCoords.x + 0.5f,
					blockCoords.y + 0.5f,
					blockCoords.z + 0.5f
				);

				// 计算从方块中心到角点的向量
				Vec3 toCorner = worldCorner - blockCenter;

				// 计算在每个轴上的穿透深度
				// 方块是1x1x1的立方体，所以半径是0.5
				float penetrationX = 0.5f - fabsf(toCorner.x);
				float penetrationY = 0.5f - fabsf(toCorner.y);
				float penetrationZ = 0.5f - fabsf(toCorner.z);

				// 找出穿透最小的轴（这是推出的最佳方向）
				if (penetrationX < penetrationY && penetrationX < penetrationZ)
				{
					// X轴穿透最小
					float pushDistance = penetrationX + DEPENETRATION_EPSILON;
					totalPushOut.x += (toCorner.x > 0.f) ? pushDistance : -pushDistance;
				}
				else if (penetrationY < penetrationZ)
				{
					// Y轴穿透最小
					float pushDistance = penetrationY + DEPENETRATION_EPSILON;
					totalPushOut.y += (toCorner.y > 0.f) ? pushDistance : -pushDistance;
				}
				else
				{
					// Z轴穿透最小
					float pushDistance = penetrationZ + DEPENETRATION_EPSILON;
					totalPushOut.z += (toCorner.z > 0.f) ? pushDistance : -pushDistance;
				}
			}
		}

		// 如果没有发现穿透，提前退出
		if (!foundPenetration)
		{
			break;
		}

		// 应用平均推出向量
		// 除以角点数量以避免过度推出
		Vec3 avgPushOut = totalPushOut / static_cast<float>(corners.size());

		// 只应用显著的推出
		if (avgPushOut.GetLengthSquared() > DEPENETRATION_EPSILON * DEPENETRATION_EPSILON)
		{
			m_position += avgPushOut;

			// 清零对应轴的速度，防止继续陷入
			if (fabsf(avgPushOut.x) > DEPENETRATION_EPSILON)
			{
				m_velocity.x = 0.f;
			}
			if (fabsf(avgPushOut.y) > DEPENETRATION_EPSILON)
			{
				m_velocity.y = 0.f;
			}
			if (fabsf(avgPushOut.z) > DEPENETRATION_EPSILON)
			{
				m_velocity.z = 0.f;
			}
		}
		else
		{
			// 推出量太小，认为已经修正完成
			break;
		}
	}
}
