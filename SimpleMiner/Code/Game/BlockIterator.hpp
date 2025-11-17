#pragma once
#include "Engine/Math/IntVec3.hpp"
class Chunk;
class Block;

class BlockIterator
{
public:
	BlockIterator() {}
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
	bool IsSolid() const;
	Block GetBlock() const;    
	Block* GetBlockPtr();
	IntVec3 GetLocalCoords() const; 
	Chunk* GetChunk() const;
	int GetBlockIndex() const;

private:
	Chunk* m_chunk = nullptr;
	int m_blockIndex = 0;
};