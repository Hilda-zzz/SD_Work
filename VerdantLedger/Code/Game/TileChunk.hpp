#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"

class TileChunk
{
public:
	TileChunk();
	~TileChunk();

	void InitializeChunkVerts();

	IntVec2 GetPosition() const { return m_startPosition; }
	IntVec2 GetSize() const { return m_size; }
	bool IsEmpty() const { return m_isEmpty; }

	uint32_t GetTile(int localX, int localY) const;
	void SetTile(int localX, int localY, uint32_t tileId);

	IntVec2 ToWorldPos(int localX, int localY) const {
		return IntVec2(m_startPosition.x * m_size.x + localX, m_startPosition.y * m_size.y + localY);
	}
public:
	IntVec2 m_startPosition;     
	IntVec2 m_size;          
	std::vector<uint32_t> m_data;
	bool m_isEmpty = false;   
	std::vector<Vertex_PCU> m_verts;

private:
	
};