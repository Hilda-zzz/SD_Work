#include "BlockIterator.hpp"
#include "Chunk.hpp"
#include "BlockDefinition.hpp"

BlockIterator BlockIterator::GetFwdX() const
{
	int x = m_chunk->IndexToLocalX(m_blockIndex);
	if (x >= CHUNK_MAX_X)
	{
		if (m_chunk->m_neighborEast)
		{
			int newIndex = m_blockIndex - CHUNK_MAX_X;
			return BlockIterator(m_chunk->m_neighborEast, newIndex);
		}
		return BlockIterator(nullptr, -1);
	}
	return BlockIterator(m_chunk, m_blockIndex + 1);
}

BlockIterator BlockIterator::GetNegX() const
{
	int x = m_chunk->IndexToLocalX(m_blockIndex);
	if (x <= 0)
	{
		if (m_chunk->m_neighborWest)
		{
			int newIndex = m_blockIndex + CHUNK_MAX_X;
			return BlockIterator(m_chunk->m_neighborWest, newIndex);
		}
		return BlockIterator(nullptr, -1);
	}
	return BlockIterator(m_chunk, m_blockIndex - 1);
}

BlockIterator BlockIterator::GetFwdY() const
{
	int y = m_chunk->IndexToLocalY(m_blockIndex);
	if (y >= CHUNK_MAX_Y)
	{
		if (m_chunk->m_neighborNorth)
		{
			int newIndex = m_blockIndex - (CHUNK_MAX_Y << CHUNK_BITS_X);  
			return BlockIterator(m_chunk->m_neighborNorth, newIndex);
		}
		return BlockIterator(nullptr, -1);
	}
	return BlockIterator(m_chunk, m_blockIndex + CHUNK_SIZE_X);
}

BlockIterator BlockIterator::GetNegY() const
{
	int y = m_chunk->IndexToLocalY(m_blockIndex);
	if (y <= 0)
	{
		if (m_chunk->m_neighborSouth)
		{
			int newIndex = m_blockIndex + (CHUNK_MAX_Y << CHUNK_BITS_X);
			return BlockIterator(m_chunk->m_neighborSouth, newIndex);
		}
		return BlockIterator(nullptr, -1);
	}
	return BlockIterator(m_chunk, m_blockIndex - CHUNK_SIZE_X);
}

BlockIterator BlockIterator::GetFwdZ() const
{
	int z = m_chunk->IndexToLocalZ(m_blockIndex);
	if (z >= CHUNK_MAX_Z)
	{
		return BlockIterator(nullptr, -1);
	}
		
	return BlockIterator(m_chunk, m_blockIndex + (CHUNK_SIZE_X * CHUNK_SIZE_Y));
}

BlockIterator BlockIterator::GetNegZ() const
{
	int z = m_chunk->IndexToLocalZ(m_blockIndex);
	if (z <= 0) return BlockIterator(nullptr, -1);
	return BlockIterator(m_chunk, m_blockIndex - (CHUNK_SIZE_X * CHUNK_SIZE_Y));
}

bool BlockIterator::IsValid() const
{
	if (m_chunk == nullptr) {
		return false;
	}
	return m_chunk->IsValidIndex(m_blockIndex);
}

bool BlockIterator::IsOpaque() const
{
	return BlockDefinition::s_blockDefs[GetBlock().GetTypeIndex()].m_isOpaque;
}

Block BlockIterator::GetBlock() const
{
	return m_chunk->GetBlock(m_blockIndex);
}

Block* BlockIterator::GetBlockPtr()
{
	return &m_chunk->GetBlockRef(m_blockIndex);
}

IntVec3 BlockIterator::GetLocalCoords() const
{
	return m_chunk->IndexToLocalCoords(m_blockIndex);
}

Chunk* BlockIterator::GetChunk() const
{
	return m_chunk;
}

int BlockIterator::GetBlockIndex() const
{
	return m_blockIndex;
}
