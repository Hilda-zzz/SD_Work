#pragma once
#include <cstdint>

class Block
{
private:
	uint8_t m_typeIndex;  

public:
	Block(uint8_t typeIndex = 0) : m_typeIndex(typeIndex) {}

	uint8_t GetTypeIndex() const { return m_typeIndex; }
	void SetTypeIndex(uint8_t typeIndex) { m_typeIndex = typeIndex; }
};