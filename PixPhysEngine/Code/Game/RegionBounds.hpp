#pragma once
#include "Engine/Math/IntVec2.hpp"

// 使用SuperChunk坐标的AABB
struct RegionBounds 
{
    IntVec2 m_bottomLeftSC;   
    IntVec2 m_topRightSC;    
    
    RegionBounds() 
        : m_bottomLeftSC(0, 0)
        , m_topRightSC(0, 0) 
    {}
    
    RegionBounds(IntVec2 const& bottomLeft, IntVec2 const& topRight)
        : m_bottomLeftSC(bottomLeft)
        , m_topRightSC(topRight)
    {}
    
    // SuperChunk units
    IntVec2 GetSizeInSuperChunks() const 
    {
        return IntVec2(
            m_topRightSC.x - m_bottomLeftSC.x + 1,
            m_topRightSC.y - m_bottomLeftSC.y + 1
        );
    }
    
    bool ContainsSuperChunk(IntVec2 const& scCoords) const 
    {
        return scCoords.x >= m_bottomLeftSC.x && scCoords.x <= m_topRightSC.x &&
               scCoords.y >= m_bottomLeftSC.y && scCoords.y <= m_topRightSC.y;
    }
    
    bool ContainsChunk(IntVec2 const& chunkCoords, int chunksPerSuperChunk) const;
    
    // 用于std::map的比较运算符
    bool operator<(const RegionBounds& other) const 
    {
        if (m_bottomLeftSC.y != other.m_bottomLeftSC.y)
            return m_bottomLeftSC.y < other.m_bottomLeftSC.y;
        return m_bottomLeftSC.x < other.m_bottomLeftSC.x;
    }
    
    bool operator==(const RegionBounds& other) const 
    {
        return m_bottomLeftSC == other.m_bottomLeftSC && 
               m_topRightSC == other.m_topRightSC;
    }
    
    bool operator!=(const RegionBounds& other) const 
    {
        return !(*this == other);
    }
};
