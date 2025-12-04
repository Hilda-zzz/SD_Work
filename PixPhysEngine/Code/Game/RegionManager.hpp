#pragma once
#include "RegionBounds.hpp"
#include "RegionGenerationData_v2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <map>
#include <vector>
#include <memory>

class HerringboneMapGenerator;
class HerringboneTileset;

// Region管理器 - 负责Region的创建、缓存和查找
class RegionManager 
{
public:
	RegionManager(unsigned int worldSeed);
	~RegionManager();

	// 初始化世界布局
	void InitializeWorldLayout();

	// ⭐ 新增：预生成所有Region
	void PreGenerateAllRegions();

	// 设置tileset（简化版本，所有Region用同一个tileset）
	void SetTileset(HerringboneTileset* tileset) { m_defaultTileset = tileset; }

	// 设置/获取Generator（可选，允许外部提供）
	void SetGenerator(HerringboneMapGenerator* generator);
	HerringboneMapGenerator* GetGenerator() const { return m_generator; }

	// 根据SuperChunk坐标查找所属的Region
	RegionBounds* FindRegionForSuperChunk(IntVec2 const& scCoords);

	// 根据Chunk坐标查找所属的Region
	RegionBounds* FindRegionForChunk(IntVec2 const& chunkCoords);

	// 获取或生成Region数据
	RegionGenerationData* GetOrCreateRegion(RegionBounds const& bounds);

	// 根据chunk坐标获取对应的Region数据
	RegionGenerationData* GetRegionDataForChunk(IntVec2 const& chunkCoords);

	// 卸载不需要的Region（LRU缓存）
	void UnloadDistantRegions(IntVec2 const& centerSC, int keepRadius = 2);

	// 调试信息
	int GetLoadedRegionCount() const { return static_cast<int>(m_regions.size()); }
	void PrintWorldLayout() const;

	const std::map<RegionBounds, std::unique_ptr<RegionGenerationData>>& GetAllRegions() const {
		return m_regions;
	}

private:
	// 生成Region的确定性种子
	unsigned int HashRegionBounds(RegionBounds const& bounds) const;

private:
	unsigned int m_worldSeed;

	// 已生成的Region数据（缓存）
	std::map<RegionBounds, std::unique_ptr<RegionGenerationData>> m_regions;

	// 世界布局 - 预定义的Region边界
	std::vector<RegionBounds> m_worldLayout;

	// Generator（共享的工具）
	HerringboneMapGenerator* m_generator;
	bool m_ownsGenerator=true;  // 是否拥有generator（决定是否需要delete）

	// 默认tileset（简化版本）
	HerringboneTileset* m_defaultTileset;
};