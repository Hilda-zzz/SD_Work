#pragma once
#include <string>
#include "Engine/Math/IntVec2.hpp"

class Tileset
{
private:
	std::string m_name;
	int m_firstGid;              // 起始GID
	int m_tileCount;             // 瓦片总数
	IntVec2 m_tileSize;          // 瓦片尺寸
	int m_columns;               // 列数
	std::string m_imagePath;     // 图片路径
	IntVec2 m_imageSize;         // 图片尺寸
	int m_margin = 0;            // 边距
	int m_spacing = 0;           // 间距

	// 瓦片属性（稀疏存储）
	//std::unordered_map<int, TileProperties> m_tileProperties;

public:
	const std::string& GetName() const { return m_name; }
	int GetFirstGid() const { return m_firstGid; }
	int GetTileCount() const { return m_tileCount; }
	const std::string& GetImagePath() const { return m_imagePath; }

	// 检查GID是否属于这个tileset
	bool ContainsGid(uint32_t gid) const {
		return gid >= m_firstGid && gid < m_firstGid + m_tileCount;
	}

	// 计算瓦片在图片中的位置
	IntVec2 GetTileTextureCoord(uint32_t gid) const;

	// 获取瓦片属性
	//const TileProperties* GetTileProperties(uint32_t gid) const;
};