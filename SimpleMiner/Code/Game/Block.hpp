#pragma once
#include <cstdint>

// Bit mask constants for block flags
constexpr uint8_t BLOCK_BIT_MASK_IS_SKY = 0b00000001;           // I am non-opaque and no opaque blocks are above me
constexpr uint8_t BLOCK_BIT_MASK_IS_LIGHT_DIRTY = 0b00000010;   // A block iterator for me is currently in the dirty light queue
constexpr uint8_t BLOCK_BIT_MASK_IS_FULL_OPAQUE = 0b00000100;   // I block light, visibility, and hide my neighbors faces
constexpr uint8_t BLOCK_BIT_MASK_IS_SOLID = 0b00001000;         // Physical objects and physics raycasts collide with me
constexpr uint8_t BLOCK_BIT_MASK_IS_VISIBLE = 0b00010000;       // I cannot be skipped during chunk mesh rebuilding

class Block
{
public:
	Block(uint8_t typeIndex = 0);

	uint8_t GetTypeIndex() const { return m_typeIndex; }
	void SetTypeIndex(uint8_t typeIndex) { m_typeIndex = typeIndex; }

	// ===================================================================================

	uint8_t GetOutdoorLightInfluence() const { return (m_lightInfluence >> 4) & 0x0F; }
	void SetOutdoorLightInfluence(uint8_t influence)
	{
		m_lightInfluence = (m_lightInfluence & 0x0F) | ((influence & 0x0F) << 4);
	}

	uint8_t GetIndoorLightInfluence() const { return m_lightInfluence & 0x0F; }
	void SetIndoorLightInfluence(uint8_t influence)
	{
		m_lightInfluence = (m_lightInfluence & 0xF0) | (influence & 0x0F);
	}

	// ===================================================================================

	bool IsSky() const {
		return m_bitFlags & BLOCK_BIT_MASK_IS_SKY;
	}
	void SetIsSky(bool isSky)
	{
		if (isSky)
			m_bitFlags |= BLOCK_BIT_MASK_IS_SKY;
		else
			m_bitFlags &= ~BLOCK_BIT_MASK_IS_SKY;
	}
	// ----------------------------------------------

	bool IsLightDirty() const { return (m_bitFlags & BLOCK_BIT_MASK_IS_LIGHT_DIRTY) != 0; }
	void SetIsLightDirty(bool isDirty)
	{
		if (isDirty)
			m_bitFlags |= BLOCK_BIT_MASK_IS_LIGHT_DIRTY;
		else
			m_bitFlags &= ~BLOCK_BIT_MASK_IS_LIGHT_DIRTY;
	}
	// ----------------------------------------------

	bool IsFullOpaque() const { return (m_bitFlags & BLOCK_BIT_MASK_IS_FULL_OPAQUE) != 0; }
	void SetIsFullOpaque(bool isOpaque)
	{
		if (isOpaque)
			m_bitFlags |= BLOCK_BIT_MASK_IS_FULL_OPAQUE;
		else
			m_bitFlags &= ~BLOCK_BIT_MASK_IS_FULL_OPAQUE;
	}
	// ----------------------------------------------

	bool IsSolid() const { return (m_bitFlags & BLOCK_BIT_MASK_IS_SOLID) != 0; }
	void SetIsSolid(bool isSolid)
	{
		if (isSolid)
			m_bitFlags |= BLOCK_BIT_MASK_IS_SOLID;
		else
			m_bitFlags &= ~BLOCK_BIT_MASK_IS_SOLID;
	}
	// ----------------------------------------------

	bool IsVisible() const { return (m_bitFlags & BLOCK_BIT_MASK_IS_VISIBLE) != 0; }
	void SetIsVisible(bool isVisible)
	{
		if (isVisible)
			m_bitFlags |= BLOCK_BIT_MASK_IS_VISIBLE;
		else
			m_bitFlags &= ~BLOCK_BIT_MASK_IS_VISIBLE;
	}
	// ----------------------------------------------
private:
	void InitializeFlagsFromDefinition();
private:
	uint8_t m_typeIndex;
	uint8_t m_lightInfluence = 0;
	uint8_t m_bitFlags = 0;
};