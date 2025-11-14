#pragma once


#include "Engine/Math/RaycastUtils.hpp"
#include "Game/BlockIterator.hpp"

struct GameRaycastResult3D : public RaycastResult3D
{
public:
	GameRaycastResult3D() {}
	GameRaycastResult3D(Vec3 fwdNormal, Vec3 startPos, float len)
		: RaycastResult3D(fwdNormal, startPos, len) {}

public:
	BlockIterator m_impactedBlockIter;  // 被击中的方块迭代器
	int m_impactedFaceIndex = -1;       // 被击中的面索引 (0-5: -X, +X, -Y, +Y, -Z, +Z)
};
