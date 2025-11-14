#include "Game/SampleImageUtils.hpp"
#include <cmath>

// 变换坐标 (将旋转/对称后的虚拟图片坐标映射回原始图片坐标)
// virtualX, virtualY: 虚拟图片上的坐标
// texDims: 原始图片的尺寸
// 返回: 原始图片上的坐标
IntVec2 TransformCoordinates(
	const IntVec2& texDims,
	int virtualX,
	int virtualY,
	Rotation rotation,
	Symmetry symmetry
) {
	// 步骤1: 应用旋转变换（从虚拟坐标映射到原始坐标）
	int texX, texY;

	switch (rotation) {
	case Rotation::ROTATE_90:
		// 旋转90度后: 虚拟图片是 height×width
		// 虚拟坐标 (vx, vy) -> 原始坐标 (vy, width-1-vx)
		texX = virtualY;
		texY = texDims.y - 1 - virtualX;
		break;

	case Rotation::ROTATE_180:
		// 旋转180度后: 虚拟图片仍是 width×height
		// 虚拟坐标 (vx, vy) -> 原始坐标 (width-1-vx, height-1-vy)
		texX = texDims.x - 1 - virtualX;
		texY = texDims.y - 1 - virtualY;
		break;

	case Rotation::ROTATE_270:
		// 旋转270度后: 虚拟图片是 height×width
		// 虚拟坐标 (vx, vy) -> 原始坐标 (height-1-vy, vx)
		//texX = texDims.y - 1 - virtualY;
		//texY = virtualX;
		texX = texDims.x - 1 - virtualY;
		texY = virtualX;
		break;

	case Rotation::ROTATE_0:
	default:
		// 无旋转
		texX = virtualX;
		texY = virtualY;
		break;
	}

	// 步骤2: 应用对称变换（在原始图片坐标系中）
	switch (symmetry) {
	case Symmetry::FLIP_X:
		// 水平翻转
		texX = texDims.x - 1 - texX;
		break;

	case Symmetry::FLIP_Y:
		// 垂直翻转
		texY = texDims.y - 1 - texY;
		break;

	case Symmetry::FLIP_BOTH:
		// 双向翻转
		texX = texDims.x - 1 - texX;
		texY = texDims.y - 1 - texY;
		break;

	case Symmetry::NONE:
	default:
		break;
	}

	// 边界检查
	if (texX < 0) texX = 0;
	if (texY < 0) texY = 0;
	if (texX >= texDims.x) texX = texDims.x - 1;
	if (texY >= texDims.y) texY = texDims.y - 1;

	return IntVec2(texX, texY);
}

// 增强版图片采样 - 支持旋转和对称
Rgba8 SampleImage(
	Image* img,
	int worldX,
	int worldY,
	float tilingMultiplier,
	Rotation rotation,
	Symmetry symmetry
) {
	if (!img) return Rgba8::WHITE;

	IntVec2 texDims = img->GetDimensions();

	// 确定旋转后的虚拟尺寸
	IntVec2 virtualDims = texDims;
	if (rotation == Rotation::ROTATE_90 || rotation == Rotation::ROTATE_270) {
		// 90度和270度旋转时，宽高互换
		virtualDims = IntVec2(texDims.y, texDims.x);
	}

	// 应用tiling系数
	float scaledX = worldX * tilingMultiplier;
	float scaledY = worldY * tilingMultiplier;

	// 转换为整数坐标（基于虚拟尺寸）
	int virtualX = static_cast<int>(floorf(scaledX)) % virtualDims.x;
	int virtualY = static_cast<int>(floorf(scaledY)) % virtualDims.y;

	// 处理负坐标
	if (virtualX < 0) virtualX += virtualDims.x;
	if (virtualY < 0) virtualY += virtualDims.y;

	// 应用旋转和对称变换，得到原始图片的坐标
	IntVec2 transformedCoords = TransformCoordinates(
		texDims,
		virtualX,
		virtualY,
		rotation,
		symmetry
	);

	return img->GetTexelColor(transformedCoords);
}

// 便捷函数: 仅旋转
Rgba8 SampleImageRotated(
	Image* img,
	int worldX,
	int worldY,
	float tilingMultiplier,
	int rotationDegrees
) {
	Rotation rot;
	switch (rotationDegrees) {
	case 90:  rot = Rotation::ROTATE_90;  break;
	case 180: rot = Rotation::ROTATE_180; break;
	case 270: rot = Rotation::ROTATE_270; break;
	default:  rot = Rotation::ROTATE_0;   break;
	}

	return SampleImage(img, worldX, worldY, tilingMultiplier, rot, Symmetry::NONE);
}

// 便捷函数: 仅对称
Rgba8 SampleImageSymmetry(
	Image* img,
	int worldX,
	int worldY,
	float tilingMultiplier,
	bool flipX,
	bool flipY
) {
	Symmetry sym;
	if (flipX && flipY) {
		sym = Symmetry::FLIP_BOTH;
	}
	else if (flipX) {
		sym = Symmetry::FLIP_X;
	}
	else if (flipY) {
		sym = Symmetry::FLIP_Y;
	}
	else {
		sym = Symmetry::NONE;
	}

	return SampleImage(img, worldX, worldY, tilingMultiplier, Rotation::ROTATE_0, sym);
}