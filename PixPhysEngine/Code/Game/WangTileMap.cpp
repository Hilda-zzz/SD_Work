#include "WangTileMap.hpp"
#include "CellMatManager.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/SandboxPlayer.hpp"
#include <cmath>
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern Renderer* g_theRenderer;

WangTileMap::WangTileMap(SandboxPlayer* player)
	: BaseMap(IntVec2(WANG_MAP_CHUNKS_X* WANG_CHUNK_SIZE, WANG_MAP_CHUNKS_Y* WANG_CHUNK_SIZE), WANG_CHUNK_SIZE)
{
	m_player = player;
	//m_player->SetCurMap(this);
	Initialize();
	LoadTemplateFromFile("Data/Images/XShapeBW.png");
	GenerateMapFromTemplate();
}

WangTileMap::~WangTileMap()
{
	BaseMap::~BaseMap();
	if (m_templateImage) {
		delete m_templateImage;
		m_templateImage = nullptr;
	}
}

void WangTileMap::Initialize()
{
	CreateChunkGrid();
}

void WangTileMap::LoadTemplateFromFile(const std::string& filename)
{
	LoadGrayscaleTemplate(filename);
}

Rgba8 WangTileMap::GetCellDebugColor(const Cell& cell, IntVec2 const& worldCoords) const
{
	return Rgba8();
}

void WangTileMap::LoadGrayscaleTemplate(const std::string& filename)
{
	m_templateImage = new Image(filename.c_str());
	IntVec2 dimensions = m_templateImage->GetDimensions();
	m_templateSize = dimensions;

	m_grayscaleTemplate.resize(dimensions.y);
	for (int y = 0; y < dimensions.y; ++y) {
		m_grayscaleTemplate[y].resize(dimensions.x);
		for (int x = 0; x < dimensions.x; ++x) {
			Rgba8 color = m_templateImage->GetTexelColor(IntVec2(x, y));

			// 转换为灰度 (Luminance = 0.299*R + 0.587*G + 0.114*B)
			float luminance = (0.299f * color.r + 0.587f * color.g + 0.114f * color.b) / 255.0f;
			m_grayscaleTemplate[y][x] = luminance;
		}
	}

	DebuggerPrintf("Loaded template: %d×%d pixels\n", dimensions.x, dimensions.y);
	DebuggerPrintf("Will generate map: %d×%d cells (%d×%d chunks)\n",
		m_mapSize.x, m_mapSize.y, m_chunkGridSize.x, m_chunkGridSize.y);
}

void WangTileMap::GenerateMapFromTemplate()
{
	GUARANTEE_OR_DIE(!m_grayscaleTemplate.empty(), "Template not loaded!");

	// 遍历所有chunks，生成每个chunk的cells
	for (int chunkY = 0; chunkY < m_chunkGridSize.y; ++chunkY) {
		for (int chunkX = 0; chunkX < m_chunkGridSize.x; ++chunkX) {
			CellChunk* chunk = m_chunks[chunkY][chunkX];
			GenerateCellsInChunk(chunk);
			chunk->RebuildVertex();
		}
	}

	DebuggerPrintf("Map generation complete!\n");
}

void WangTileMap::GenerateCellsInChunk(CellChunk* chunk)
{
	IntVec2 chunkCoords = chunk->GetChunkIndex();

	for (int localY = 0; localY < WANG_CHUNK_SIZE; ++localY) {
		for (int localX = 0; localX < WANG_CHUNK_SIZE; ++localX) {
			int worldX = chunkCoords.x * WANG_CHUNK_SIZE + localX;
			int worldY = chunkCoords.y * WANG_CHUNK_SIZE + localY;

			float density = CalculateCellDensity(worldX, worldY);

			Cell& cell = chunk->GetLocalCell(localX, localY);

			// 简单阈值: density > 0.5 = 石头，否则为空
			if (density > 0.5f) 
			{
				cell.SetToType(CellMatType::MAT_STONE);
			}
			else 
			{
				cell.SetEmpty();
			}
		}
	}
}

float WangTileMap::CalculateCellDensity(int cellX, int cellY) const {
	// 步骤1: 映射到模板坐标 (浮点)
	float templateX = cellX / static_cast<float>(WANG_CELLS_PER_PIXEL);
	float templateY = cellY / static_cast<float>(WANG_CELLS_PER_PIXEL);

	// 步骤2: 找到周围4个像素的索引
	int x0 = static_cast<int>(std::floor(templateX));
	int y0 = static_cast<int>(std::floor(templateY));
	int x1 = std::min(x0 + 1, m_templateSize.x - 1);
	int y1 = std::min(y0 + 1, m_templateSize.y - 1);

	// 边界检查
	x0 = std::clamp(x0, 0, m_templateSize.x - 1);
	y0 = std::clamp(y0, 0, m_templateSize.y - 1);

	// 步骤3: 计算插值参数 (小数部分)
	float fx = templateX - x0;
	float fy = templateY - y0;

	// 步骤4: 获取4个像素的灰度值
	float v00 = GetTemplateDensityAt(x0, y0);  // 左下
	float v10 = GetTemplateDensityAt(x1, y0);  // 右下
	float v11 = GetTemplateDensityAt(x1, y1);  // 右上
	float v01 = GetTemplateDensityAt(x0, y1);  // 左上

	// 步骤5: 双线性插值
	float v0 = v00 * (1.0f - fx) + v10 * fx;  // 下边插值
	float v1 = v01 * (1.0f - fx) + v11 * fx;  // 上边插值
	float density = v0 * (1.0f - fy) + v1 * fy;  // 最终插值

	return density;
}

float WangTileMap::GetTemplateDensityAt(int templateX, int templateY) const {
	if (templateX < 0 || templateX >= m_templateSize.x ||
		templateY < 0 || templateY >= m_templateSize.y) {
		return 0.0f;
	}

	return m_grayscaleTemplate[templateY][templateX];
}

void WangTileMap::Update(float deltaTime) 
{
	m_deltaTime = deltaTime;
}

void WangTileMap::Render() const
{
	// === Chunk Grid Lines (Gray) ===

	g_theRenderer->BeginCamera(m_player->m_camera);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);

	for (const auto& row : m_chunks) {
	for (const auto& chunk : row) {
		chunk->RenderChunk();
	}
}

	std::vector<Vertex_PCU> chunkGridVerts;

	// Vertical lines
	for (int chunkX = 0; chunkX <= m_chunkGridSize.x; ++chunkX) {
		float worldX = static_cast<float>(chunkX * CHUNK_SIZE);
		Vec2 start(worldX, 0.f);
		Vec2 end(worldX, static_cast<float>(m_mapSize.y));
		Rgba8 gridColor(100, 100, 100, 128);
		AddVertsForLinSegment2D(chunkGridVerts, start, end, 0.5f, Rgba8::MAGNETA);
		AddVertsForLinSegment2D(chunkGridVerts, start+Vec2(32.f,0.f), end + Vec2(32.f, 0.f), 0.5f, gridColor);
	}

	// Horizontal lines
	for (int chunkY = 0; chunkY <= m_chunkGridSize.y; ++chunkY) {
		float worldY = static_cast<float>(chunkY * CHUNK_SIZE);
		Vec2 start(0.f, worldY);
		Vec2 end(static_cast<float>(m_mapSize.x), worldY);
		Rgba8 gridColor(100, 100, 100, 128);
		AddVertsForLinSegment2D(chunkGridVerts, start, end, 0.5f, Rgba8::MAGNETA);
		AddVertsForLinSegment2D(chunkGridVerts, start + Vec2(0.f, 32.f), end + Vec2(0.f, 32.f), 0.5f, gridColor);
	}

	g_theRenderer->DrawVertexArray(chunkGridVerts);

}