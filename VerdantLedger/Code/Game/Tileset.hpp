﻿#pragma once
#include <string>
#include <unordered_map>
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

class Texture;

struct TileProperty
{
	uint32_t m_localID;
	std::string m_propertyName;
	bool m_value=false;
};

class Tileset
{
public:
	Tileset(XmlElement* rootElement);
	~Tileset();

	const std::string& GetName() const { return m_name; }
	int GetFirstGid() const { return m_firstGid; }
	void SetFirstGid(int firstGid) { m_firstGid = firstGid; }
	int GetTileCount() const { return m_tileCount; }
	const std::string& GetImagePath() const { return m_imagePath; }

	AABB2 GetTileUVByInnerIndex(int index) const;
	bool ContainsGid(uint32_t gid) const {
		return gid >= m_firstGid && gid < m_firstGid + m_tileCount;
	}

	// 计算瓦片在图片中的位置
	IntVec2 GetTileTextureCoord(uint32_t gid) const;

	// 获取瓦片属性
	//const TileProperties* GetTileProperties(uint32_t gid) const;
private:


public:
	std::vector<TileProperty> m_properties;

private:
	std::string m_name;
	uint32_t m_firstGid=0;              // 起始GID
	int m_tileCount=0;            
	IntVec2 m_tileSize=IntVec2(0,0);          
	int m_columns=0;            
	std::string m_imagePath;    
	Texture* m_texture = nullptr;
	SpriteSheet* m_spriteSheet;
	IntVec2 m_imageSize = IntVec2(0, 0);         // 图片尺寸
	int m_margin = 0;            // 边距
	int m_spacing = 0;           // 间距

	// 瓦片属性（稀疏存储）
	//std::unordered_map<std::string, bool> m_tileProperties; //property name to bool
	


};