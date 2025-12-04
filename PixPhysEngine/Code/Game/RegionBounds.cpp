#include "RegionBounds.hpp"

bool RegionBounds::ContainsChunk(IntVec2 const& chunkCoords, int chunksPerSuperChunk) const
{
    // Chunk to Super Chunk
    IntVec2 scCoords(
        chunkCoords.x / chunksPerSuperChunk,
        chunkCoords.y / chunksPerSuperChunk
    );
    
    return ContainsSuperChunk(scCoords);
}
