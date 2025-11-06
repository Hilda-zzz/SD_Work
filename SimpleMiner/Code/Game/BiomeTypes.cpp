#include "BiomeTypes.hpp"

Rgba8 GetLevelColor(int level, int maxLevels)
{
	if (maxLevels <= 1)
	{
		return Rgba8::WHITE;
	}
	float t = static_cast<float>(level) / static_cast<float>(maxLevels - 1);
	unsigned char grayValue = static_cast<unsigned char>(t * 255.0f);

	return Rgba8(grayValue, grayValue, grayValue, 255);
}

Rgba8 GetBiomeColor(BiomeType biomeType)
{
	int index = static_cast<int>(biomeType);
	int totalBiomes = static_cast<int>(BiomeType::COUNT);

	if (index >= 0 && index < totalBiomes)
	{
		// 均匀分布灰度值
		float t = static_cast<float>(index) / static_cast<float>(totalBiomes - 1);
		unsigned char grayValue = static_cast<unsigned char>(t * 255.0f);
		return Rgba8(grayValue, grayValue, grayValue, 255);
	}

	// 错误情况返回洋红色（便于识别）
	return Rgba8::MAGNETA;
}
