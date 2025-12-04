#include "RegionManager.hpp"
#include "HerringboneMapGenerator_v2.hpp"
#include "HerringboneTileset.hpp"
#include "GameCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "SuperChunk.hpp"

RegionManager::RegionManager(unsigned int worldSeed)
	: m_worldSeed(worldSeed)
	, m_generator(nullptr)
	, m_ownsGenerator(true)  // 默认自己创建，自己拥有
	, m_defaultTileset(nullptr)
{
	// 创建共享的Generator（无状态，可复用）
	m_generator = new HerringboneMapGenerator();

	DebuggerPrintf("=== RegionManager Created ===\n");
	DebuggerPrintf("World Seed: %u\n", m_worldSeed);
}

RegionManager::~RegionManager()
{
	// 清空所有Region
	m_regions.clear();

	// 如果拥有generator，则删除
	if (m_ownsGenerator && m_generator) {
		delete m_generator;
		m_generator = nullptr;
	}

	DebuggerPrintf("=== RegionManager Destroyed ===\n");
}

void RegionManager::SetGenerator(HerringboneMapGenerator* generator)
{
	// 如果之前拥有generator，先删除
	if (m_ownsGenerator && m_generator) {
		delete m_generator;
	}

	m_generator = generator;
	m_ownsGenerator = false;  // 外部提供的，不拥有

	DebuggerPrintf("RegionManager: Using external generator\n");
}

void RegionManager::InitializeWorldLayout()
{
	m_worldLayout.clear();

	// 示例：定义一个简单的3×2世界
	// 每个Region是4×4 SuperChunks

	// Region 0: 左下 (起始区域)
	m_worldLayout.push_back(RegionBounds{
		IntVec2(0, 0),   // bottomLeft SC
		IntVec2(10, 5)    // topRight SC (包含，所以是4×4)
		});

	//// Region 1: 中下
	//m_worldLayout.push_back(RegionBounds{
	//	IntVec2(4, 0),
	//	IntVec2(7, 3)
	//	});

	//// Region 2: 右下
	//m_worldLayout.push_back(RegionBounds{
	//	IntVec2(8, 0),
	//	IntVec2(11, 3)
	//	});

	//// Region 3: 左上
	//m_worldLayout.push_back(RegionBounds{
	//	IntVec2(0, 4),
	//	IntVec2(3, 7)
	//	});

	//// Region 4: 中上
	//m_worldLayout.push_back(RegionBounds{
	//	IntVec2(4, 4),
	//	IntVec2(7, 7)
	//	});

	//// Region 5: 右上
	//m_worldLayout.push_back(RegionBounds{
	//	IntVec2(8, 4),
	//	IntVec2(11, 7)
	//	});

	DebuggerPrintf("=== World Layout Initialized ===\n");
	DebuggerPrintf("Total Regions: %d\n", static_cast<int>(m_worldLayout.size()));
	PrintWorldLayout();
}

void RegionManager::PreGenerateAllRegions()
{
	int successCount = 0;

	for (RegionBounds const& regionBound : m_worldLayout)
	{
		// 检查是否已存在
		if (m_regions.find(regionBound) != m_regions.end()) 
		{
			continue;  // 已生成，跳过
		}

		DebuggerPrintf("Generating region SC[%d,%d]-SC[%d,%d]\n",
			regionBound.m_bottomLeftSC.x, regionBound.m_bottomLeftSC.y,
			regionBound.m_topRightSC.x, regionBound.m_topRightSC.y);

		// 创建Region
		auto region = std::make_unique<RegionGenerationData>();
		region->SetBounds(regionBound);
		region->SetSeed(HashRegionBounds(regionBound));
		region->SetTileset(m_defaultTileset);

		// ⭐ 生成数据
		region->Generate(m_generator);

		// ⭐ 存储到m_regions
		m_regions[regionBound] = std::move(region);
		successCount++;
	}

	DebuggerPrintf("Pre-generated %d regions, total loaded: %d\n\n",
		successCount, GetLoadedRegionCount());
}

RegionBounds* RegionManager::FindRegionForSuperChunk(IntVec2 const& scCoords)
{
	for (auto& bounds : m_worldLayout) {
		if (bounds.ContainsSuperChunk(scCoords)) {
			return &bounds;
		}
	}

	// 未找到
	return nullptr;
}

RegionBounds* RegionManager::FindRegionForChunk(IntVec2 const& chunkCoords)
{
	// 转换为SuperChunk坐标
	IntVec2 scCoords(
		chunkCoords.x / CHUNKS_PER_SUPER_CHUNK,
		chunkCoords.y / CHUNKS_PER_SUPER_CHUNK
	);

	return FindRegionForSuperChunk(scCoords);
}

RegionGenerationData* RegionManager::GetOrCreateRegion(RegionBounds const& bounds)
{
	// 检查是否已生成
	auto it = m_regions.find(bounds);
	if (it != m_regions.end()) {
		DebuggerPrintf("RegionManager: Using cached region SC[%d,%d]-SC[%d,%d]\n",
			bounds.m_bottomLeftSC.x, bounds.m_bottomLeftSC.y,
			bounds.m_topRightSC.x, bounds.m_topRightSC.y);
		return it->second.get();
	}

	// 创建新Region
	auto region = std::make_unique<RegionGenerationData>();
	region->SetBounds(bounds);
	region->SetSeed(HashRegionBounds(bounds));
	region->SetTileset(m_defaultTileset);

	DebuggerPrintf("\n=== Creating New Region ===\n");
	DebuggerPrintf("Bounds: SC[%d,%d] - SC[%d,%d]\n",
		bounds.m_bottomLeftSC.x, bounds.m_bottomLeftSC.y,
		bounds.m_topRightSC.x, bounds.m_topRightSC.y);
	DebuggerPrintf("Seed: %u\n", region->GetSeed());

	// ⭐ 使用无状态的Generator生成
	// Generator可以被多个Region复用
	if (!m_generator) {
		DebuggerPrintf("ERROR: No generator available!\n");
		return nullptr;
	}

	region->Generate(m_generator);

	// 存储
	RegionGenerationData* ptr = region.get();
	m_regions[bounds] = std::move(region);

	DebuggerPrintf("Total loaded regions: %d\n\n", GetLoadedRegionCount());

	return ptr;
}

RegionGenerationData* RegionManager::GetRegionDataForChunk(IntVec2 const& chunkCoords)
{
	// 查找该chunk属于哪个Region
	RegionBounds* bounds = FindRegionForChunk(chunkCoords);

	if (!bounds) {
		DebuggerPrintf("Warning: Chunk (%d,%d) is outside all defined Regions\n",
			chunkCoords.x, chunkCoords.y);
		return nullptr;
	}

	// 获取或生成该Region的数据
	return GetOrCreateRegion(*bounds);
}

void RegionManager::UnloadDistantRegions(IntVec2 const& centerSC, int keepRadius)
{
	std::vector<RegionBounds> toUnload;

	// 遍历所有已加载的Region
	for (auto& pair : m_regions) {
		const RegionBounds& bounds = pair.first;

		// 计算Region中心
		IntVec2 regionCenter(
			(bounds.m_bottomLeftSC.x + bounds.m_topRightSC.x) / 2,
			(bounds.m_bottomLeftSC.y + bounds.m_topRightSC.y) / 2
		);

		// 计算与中心的距离（使用切比雪夫距离）
		int dx = abs(regionCenter.x - centerSC.x);
		int dy = abs(regionCenter.y - centerSC.y);
		int distance = (dx > dy) ? dx : dy;

		// 如果距离超过保留半径，标记为卸载
		if (distance > keepRadius * 4) {  // 4是因为每个Region是4×4 SC
			toUnload.push_back(bounds);
		}
	}

	// 卸载远距离Region
	for (auto& bounds : toUnload) {
		DebuggerPrintf("Unloading Region SC[%d,%d]-SC[%d,%d]\n",
			bounds.m_bottomLeftSC.x, bounds.m_bottomLeftSC.y,
			bounds.m_topRightSC.x, bounds.m_topRightSC.y);
		m_regions.erase(bounds);
	}

	if (!toUnload.empty()) {
		DebuggerPrintf("Unloaded %d regions, %d remaining\n",
			static_cast<int>(toUnload.size()), GetLoadedRegionCount());
	}
}

unsigned int RegionManager::HashRegionBounds(RegionBounds const& bounds) const
{
	unsigned int hash = m_worldSeed;
	hash = hash * 31 + bounds.m_bottomLeftSC.x;
	hash = hash * 31 + bounds.m_bottomLeftSC.y;
	hash = hash * 31 + bounds.m_topRightSC.x;
	hash = hash * 31 + bounds.m_topRightSC.y;
	return hash;
}

void RegionManager::PrintWorldLayout() const
{
	DebuggerPrintf("\n=== World Layout ===\n");
	for (size_t i = 0; i < m_worldLayout.size(); ++i) {
		const RegionBounds& bounds = m_worldLayout[i];
		IntVec2 size = bounds.GetSizeInSuperChunks();
		DebuggerPrintf("Region %d: SC[%d,%d] - SC[%d,%d] (Size: %dx%d SCs)\n",
			static_cast<int>(i),
			bounds.m_bottomLeftSC.x, bounds.m_bottomLeftSC.y,
			bounds.m_topRightSC.x, bounds.m_topRightSC.y,
			size.x, size.y);
	}
	DebuggerPrintf("\n");
}
