#pragma once
#include <vector>
#include "Engine/Math/Vec2.hpp"
#include "Game/Cell.hpp"
#include <ThirdParty/box2d/include/box2d/types.h>

struct RigidBodyPrecomputedData
{
	// 原始数据
	std::vector<CellWithCoords> originalCells;  // 真实的cells
	std::vector<IntVec2> virtualFilledCoords;   // 虚拟填充的坐标（仅用于生成刚体）

	// 输入数据
	std::vector<CellWithCoords> cellsForRigidBody;
	b2BodyType bodyType;

	// 预计算的数据
	Vec2 centroid;                          // 质心
	std::vector<Vec2> outlinePoints;        // 轮廓点
	std::vector<Vec2> triangleVertices;     // 三角网格顶点（已转为局部坐标）
	std::vector<unsigned int> triangleIndices;  // 三角形索引

	// 静态刚体的cell坐标映射（可以预先构建）
	std::unordered_map<Cell*, IntVec2> staticCellCoords;

	// 动态刚体的蓝图（可以预先构建）
	std::unordered_map<IntVec2, Cell, IntVec2Hash> cellBlueprint;

	bool isValid = false;  // 数据是否有效
	bool hasVirtualFill = false;  // 是否有虚拟填充
};