#pragma once
#include "Cell.hpp"
#include "CellChunk.hpp"
#include "CellMatManager.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>

class BaseMap 
{
public:
	virtual ~BaseMap();

	// ========================================
	// Core Process
	// ========================================
	virtual void Initialize() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render() const = 0;

	// ========================================
	// Cell 通用
	// ========================================
	/// 通过世界坐标获取 Cell（跨 Chunk）
	virtual Cell& GetCell(int worldX, int worldY);
	virtual Cell const& GetCell(int worldX, int worldY) const;

	/// 检查世界坐标是否在地图范围内
	virtual bool IsInBounds(int worldX, int worldY) const;

	// ========================================
	// Chunk 访问接口（已实现，所有子类通用）
	// ========================================
	virtual CellChunk* GetChunk(int chunkX, int chunkY);
	virtual CellChunk const* GetChunk(int chunkX, int chunkY) const;

	/// 通过世界坐标获取所在的 Chunk
	virtual CellChunk* GetChunkByWorldPos(int worldX, int worldY);
	virtual const CellChunk* GetChunkByWorldPos(int worldX, int worldY) const;

	/// 检查 Chunk 索引是否有效
	virtual bool IsChunkIndexValid(int chunkX, int chunkY) const;

	// ========================================
	// 物理查询接口（已实现，供 CellBehaviorSystem 使用）
	// ========================================
	// 这些函数从 SandboxMap 提升到基类，因为它们是通用的物理规则

	/// MoveSolid 能否移动到目标位置
	/// 条件：目标为空 或 (目标是液体 且 当前密度大于目标密度)
	virtual bool MSCanMoveTo(int x, int y, float curDensity) const;

	/// Liquid 能否移动到目标位置
	/// 条件：当前密度大于目标密度（包括空气密度=0）
	virtual bool LiquidCanMoveTo(int x, int y, float curDensity) const;

	/// 检查是否能水平移动
	/// 条件：速度足够 且 有支撑
	virtual bool CanMoveHorizontally(int x, int y, const Cell& cell) const;

	/// 检查是否有支撑（下方是否有非空 Cell）
	virtual bool HasSupport(int x, int y) const;

	// ========================================
	// Get Funcs
	// ========================================
	IntVec2 GetMapSize() const { return m_mapSize; }
	IntVec2 GetChunkGridSize() const { return m_chunkGridSize; }
	AABB2 GetMapBounds() const { return m_mapBounds; }
	int GetChunkSize() const { return m_chunkSize; }
	float GetDeltaTime() const { return m_deltaTime; }

	// ========================================
	// Help Funcs
	// ========================================
	virtual Rgba8 GetCellDebugColor(const Cell& cell, IntVec2 const& worldCoords) const;

protected:
	// ========================================
	// 构造函数（仅供子类调用）
	// ========================================
	BaseMap(const IntVec2& mapSize, int chunkSize = 64);

	// ========================================
	// 工具方法（供子类使用）
	// ========================================

	/// 创建 Chunk 网格（在子类 Initialize 中调用）
	void CreateChunkGrid();

	/// 销毁 Chunk 网格（在子类析构中调用）
	void DestroyChunkGrid();

	// ========================================
	// 共享数据成员
	// ========================================

	// Chunk 管理
	std::vector<std::vector<CellChunk*>> m_chunks;  // [chunkY][chunkX]
	IntVec2 m_chunkGridSize;                        // Chunk 网格大小（列数，行数）
	int m_chunkSize;                                // 单个 Chunk 的尺寸（默认 64）

	// 地图信息
	IntVec2 m_mapSize;                              // Cell 地图大小（宽度，高度）
	AABB2 m_mapBounds;                              // 地图边界

	// 时间
	float m_deltaTime = 0.0f;
};

// ========================================
// 内联实现（简单的访问器）
// ========================================

inline bool BaseMap::IsInBounds(int worldX, int worldY) const {
	return worldX >= 0 && worldX < m_mapSize.x &&
		worldY >= 0 && worldY < m_mapSize.y;
}

inline bool BaseMap::IsChunkIndexValid(int chunkX, int chunkY) const {
	return chunkX >= 0 && chunkX < m_chunkGridSize.x &&
		chunkY >= 0 && chunkY < m_chunkGridSize.y;
}