#pragma once
#include "Engine/Math/IntVec3.hpp"
class Chunk;
class Block;

class BlockIterator
{
public:
	BlockIterator(Chunk* chunk, int index) :m_chunk(chunk), m_blockIndex(index) {};
	~BlockIterator() {}

	BlockIterator GetFwdX() const;
	BlockIterator GetNegX() const;
	BlockIterator GetFwdY() const;
	BlockIterator GetNegY() const;
	BlockIterator GetFwdZ() const;
	BlockIterator GetNegZ() const;

	bool IsValid() const;   
	bool IsOpaque() const;
	Block GetBlock() const;          
	IntVec3 GetLocalCoords() const; 

private:
	Chunk* m_chunk = nullptr;
	int m_blockIndex = 0;
};