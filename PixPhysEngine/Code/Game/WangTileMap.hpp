#pragma once
#include "BaseMap.hpp"
#include "Engine/Core/Image.hpp"
#include <string>
#include "PCGTemplate.hpp"
#include "HerringboneMapGenerator.hpp"

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

	// === Edge Detection ===
	void DetectTemplateEdges();
	bool IsCellOnEdge(int worldX, int worldY) const;
	void SetDrawEdges(bool draw) { m_drawEdges = draw; }
	bool GetDrawEdges() const { return m_drawEdges; }

private:
	// new ===================
	void LoadMultiLayerTemplate(const std::string& filename);
	void InitializeColorMaterialMappings();
	PixelLayerType ClassifyPixel(const Rgba8& m_color) const;
	CellMatType GetMaterialTypeForColor(const Rgba8& m_color) const;
	float ColorDistance(const Rgba8& c1, const Rgba8& c2) const;
	bool IsGrayscale(const Rgba8& m_color, float tolerance = 10.0f) const;

	float CalculateBaseDensity(int cellX, int cellY) const;
	float CalculateColoredDensity(int cellX, int cellY, const Rgba8& targetColor) const;
	TemplatePixel GetTemplatePixelAt(int templateX, int templateY) const;

	void GenerateBaseLayer();
	void GenerateColoredLayers();
	void GenerateBaseCellsInChunk(CellChunk* chunk);
	void GenerateColoredCellsInChunk(CellChunk* chunk, const Rgba8& layerColor, CellMatType matType);
	std::vector<Rgba8> GetUniqueColoredLayers() const;

	// === 新增：主题纹理支持 ===
	void LoadMaterialTextures();
	Rgba8 SampleMaterialTexture(CellMatType matType, int worldX, int worldY) const;
	//Rgba8 SampleImage(Image* img, int worldX, int worldY, float tilingMultiplier = 1.0f) const;
	// ===============================

	//void LoadGrayscaleTemplate(const std::string& filename);
	//float GetTemplateDensityAt(int templateX, int templateY) const;
	//float CalculateCellDensity(int cellX, int cellY) const;
	//void GenerateCellsInChunk(CellChunk* chunk);

	void ResetCellColorForDebugRender(CellChunk* chunk);

	void RenderDebugUI();
	void RenderWangTemplateOverlay() const;
	void RenderEdgePixelOverlay() const;


	// Wang Tile 特有数据
	Image* m_templateImage = nullptr;
	std::vector<std::vector<TemplatePixel>> m_templateData;  // [y][x], 值范围 [0,1]
	IntVec2 m_templateSize;

	std::vector<ColorMaterialMapping> m_colorMaterialMappings;

	SandboxPlayer* m_player = nullptr;

	bool m_showWangTemplate = false;
	bool  m_drawEdges = false;

	std::map<CellMatType, Image*> m_materialTextures;

	Image* m_edgeSticker = nullptr;
	Image* m_sand = nullptr;
	Image* m_sandEdgeSticker = nullptr;

	Image* m_stickerImageA = nullptr;               // RED区域贴画
	Image* m_stickerImageB = nullptr;               // CYAN区域贴画

	
};