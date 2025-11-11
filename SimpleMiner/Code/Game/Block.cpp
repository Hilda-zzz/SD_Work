#include "Block.hpp"
#include "BlockDefinition.hpp"

Block::Block(uint8_t typeIndex) : m_typeIndex(typeIndex)
, m_lightInfluence(0)
, m_bitFlags(0)
{
	InitializeFlagsFromDefinition();
}

void Block::InitializeFlagsFromDefinition()
{
	if (m_typeIndex >= BlockDefinition::s_blockDefs.size())
	{
		m_bitFlags = 0; 
		return;
	}

	const BlockDefinition& def = BlockDefinition::s_blockDefs[m_typeIndex];

	if (def.m_isOpaque)
	{
		m_bitFlags |= BLOCK_BIT_MASK_IS_FULL_OPAQUE;
	}

	if (def.m_isSolid)
	{
		m_bitFlags |= BLOCK_BIT_MASK_IS_SOLID;
	}

	if (def.m_isVisible)
	{
		m_bitFlags |= BLOCK_BIT_MASK_IS_VISIBLE;
	}
}
