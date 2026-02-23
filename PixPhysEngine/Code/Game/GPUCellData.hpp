#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <stdint.h>

// GPU Cell数据格式（12 bytes）
struct GPUCellData {
	uint32_t worldX;       // 4 bytes: 世界X坐标（0 ~ 65535）
	uint32_t worldY;       // 4 bytes: 世界Y坐标（0 ~ 65535）
	uint32_t colorPacked;  // 4 bytes: RGBA8888
	uint32_t attributes;   // 4 bytes: PhysicsType(2) + Emission(6) + Reserved(24)
	//uint32_t padding;

	// colorPacked布局:
	// [0-7]   : R (8 bits)
	// [8-15]  : G (8 bits)
	// [16-23] : B (8 bits)
	// [24-31] : A (8 bits)

	// attributes布局:
	// [0-1]   : PhysicsType (2 bits: 0=空, 1=静态固体, 2=动态固体, 3=液体)
	// [2-7]   : EmissionValue (6 bits: 0-63)
	// [8-31]  : Reserved (24 bits)

	GPUCellData()
		: worldX(0), worldY(0), colorPacked(0), attributes(0)
	{}

	// 打包
	static GPUCellData Pack(IntVec2 worldPos, Rgba8 color, uint8_t physicsType, uint8_t emission) {
		GPUCellData data;
		data.worldX = (uint32_t)worldPos.x;  // ← 改为uint32_t
		data.worldY = (uint32_t)worldPos.y;  // ← 改为uint32_t

		data.colorPacked = (uint32_t)(color.r) |
			((uint32_t)(color.g) << 8) |
			((uint32_t)(color.b) << 16) |
			((uint32_t)(color.a) << 24);

		data.attributes = ((uint32_t)(physicsType & 0x7)) |
			((emission & 0x1F) << 3);
		return data;
	}

	// 解包
	static void Unpack(const GPUCellData& data, IntVec2& outWorldPos, Rgba8& outColor,
		uint8_t& outPhysicsType, uint8_t& outEmission) {
		outWorldPos.x = (int)data.worldX;
		outWorldPos.y = (int)data.worldY;

		outColor.r = (uint8_t)(data.colorPacked & 0xFF);
		outColor.g = (uint8_t)((data.colorPacked >> 8) & 0xFF);
		outColor.b = (uint8_t)((data.colorPacked >> 16) & 0xFF);
		outColor.a = (uint8_t)((data.colorPacked >> 24) & 0xFF);

		outPhysicsType = (uint8_t)(data.attributes & 0x3);
		outEmission = (uint8_t)((data.attributes >> 2) & 0x3F);
	}
};