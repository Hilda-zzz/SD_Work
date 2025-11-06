#pragma once
#include "BaseMap.hpp"
#include "Engine/Core/Image.hpp"
#include <string>

class SandboxPlayer;

constexpr int WANG_CHUNK_SIZE = 64;
constexpr int WANG_CELLS_PER_PIXEL = 16;
constexpr int WANG_MAP_CHUNKS_X = 10;
constexpr int WANG_MAP_CHUNKS_Y = 5;

class WangTileMap : public BaseMap 
{
public:
	WangTileMap(SandboxPlayer* player);
	~WangTileMap() override;

	// base class interface
	void Initialize() override;
	void Update(float deltaTime) override;
	void Render() const override;

	void LoadTemplateFromFile(const std::string& filename);
	void GenerateMapFromTemplate();

	IntVec2 GetTemplateSize() const { return m_templateSize; }
	Rgba8 GetCellDebugColor(const Cell& cell, IntVec2 const& worldCoords) const override;
private:
	void LoadGrayscaleTemplate(const std::string& filename);
	float GetTemplateDensityAt(int templateX, int templateY) const;
	float CalculateCellDensity(int cellX, int cellY) const;
	void GenerateCellsInChunk(CellChunk* chunk);

	void ResetCellColorForDebugRender(CellChunk* chunk);

	// Wang Tile 特有数据
	Image* m_templateImage = nullptr;
	std::vector<std::vector<float>> m_grayscaleTemplate;  // [y][x], 值范围 [0,1]
	IntVec2 m_templateSize;

	SandboxPlayer* m_player = nullptr;
};