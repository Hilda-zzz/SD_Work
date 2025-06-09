#pragma once
#include <vector>
#include <string>
#include "Engine/Math/IntVec2.hpp"
#include "TileChunk.hpp"

enum class TileLayerType {
	TILE_LAYER,
	OBJECT_LAYER,
	IMAGE_LAYER,
	GROUP_LAYER
};

class TileLayer
{
public:
	TileLayer();
	~TileLayer();

	TileLayerType GetType() const { return m_type; }
	const std::string& GetName() const { return m_name; }
	IntVec2 GetSize() const { return m_size; }
	bool IsVisible() const { return m_visible; }
	float GetOpacity() const { return m_opacity; }

	uint32_t GetTile(int x, int y) const;
	void SetTile(int x, int y, uint32_t tileId);

	const std::vector<TileChunk>& GetChunks() const { return m_chunks; }
	TileChunk* GetChunk(int chunkX, int chunkY);

	//const std::vector<TileObject>& GetObjects() const { return m_objects; }

	std::string GetProperty(const std::string& key, const std::string& defaultValue = "") const;

public:


private:
	TileLayerType m_type = TileLayerType::TILE_LAYER;
	int m_id;
	std::string m_name;
	IntVec2 m_size;                          
	float m_opacity = 1.0f;
	bool m_visible = true;

	std::vector<TileChunk> m_chunks;         // infinite
	std::vector<uint32_t> m_data;           // finite

	//std::vector<TileObject> m_objects; 

	// ?
	//std::unordered_map<std::string, std::string> m_properties;
};