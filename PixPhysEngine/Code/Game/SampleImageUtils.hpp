#pragma once
#include "Engine/Core/Image.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"

// 旋转方向枚举 (顺时针)
enum class Rotation {
	ROTATE_0 = 0,   // 0度
	ROTATE_90 = 1,   // 90度
	ROTATE_180 = 2,   // 180度
	ROTATE_270 = 3    // 270度
};

// 对称类型枚举
enum class Symmetry {
	NONE = 0,   // 无对称
	FLIP_X = 1,   // 水平翻转 (沿Y轴)
	FLIP_Y = 2,   // 垂直翻转 (沿X轴)
	FLIP_BOTH = 3    // 双向翻转
};

// 变换坐标 (应用旋转和对称)
IntVec2 TransformCoordinates(
	const IntVec2& texDims,
	int texX,
	int texY,
	Rotation rotation,
	Symmetry symmetry
);

// 增强版图片采样 - 支持旋转和对称
Rgba8 SampleImage(
	Image* img,
	int worldX,
	int worldY,
	float tilingMultiplier=1.f,
	Rotation rotation = Rotation::ROTATE_0,
	Symmetry symmetry = Symmetry::NONE
);

// 便捷函数: 仅旋转
Rgba8 SampleImageRotated(
	Image* img,
	int worldX,
	int worldY,
	float tilingMultiplier,
	int rotationDegrees  // 0, 90, 180, 270
);

// 便捷函数: 仅对称
Rgba8 SampleImageSymmetry(
	Image* img,
	int worldX,
	int worldY,
	float tilingMultiplier,
	bool flipX,
	bool flipY
);