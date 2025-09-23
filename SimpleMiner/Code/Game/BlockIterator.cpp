#include "BlockIterator.hpp"
#include "Chunk.hpp"
#include "BlockDefinition.hpp"

BlockIterator BlockIterator::GetFwdX() const
{
	int x = m_chunk->IndexToLocalX(m_blockIndex);
	if (x >= CHUNK_MAX_X) return BlockIterator(nullptr, -1);
	return BlockIterator(m_chunk, m_blockIndex + 1);
}

BlockIterator BlockIterator::GetNegX() const
{
	int x = m_chunk->IndexToLocalX(m_blockIndex);
	if (x <= 0) return BlockIterator(nullptr, -1);
	return BlockIterator(m_chunk, m_blockIndex - 1);
}

BlockIterator BlockIterator::GetFwdY() const
{
	int y = m_chunk->IndexToLocalY(m_blockIndex);
	if (y >= CHUNK_MAX_Y) return BlockIterator(nullptr, -1);
	return BlockIterator(m_chunk, m_blockIndex + CHUNK_SIZE_X);
}

BlockIterator BlockIterator::GetNegY() const
{
	int y = m_chunk->IndexToLocalY(m_blockIndex);
	if (y <= 0) return BlockIterator(nullptr, -1);
	return BlockIterator(m_chunk, m_blockIndex - CHUNK_SIZE_X);
}

BlockIterator BlockIterator::GetFwdZ() const
{
	int z = m_chunk->IndexToLocalZ(m_blockIndex);
	if (z >= CHUNK_MAX_Z) return BlockIterator(nullptr, -1); 
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

IntVec3 BlockIterator::GetLocalCoords() const
{
	return m_chunk->IndexToLocalCoords(m_blockIndex);
}
