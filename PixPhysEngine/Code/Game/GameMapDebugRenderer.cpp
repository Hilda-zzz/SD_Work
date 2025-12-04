// GameMapDebugRenderer.cpp
#include "GameMapDebugRenderer.hpp"
#include "GameMap.hpp"
#include "SuperChunk.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/RegionManager.hpp"

extern Renderer* g_theRenderer;

GameMapDebugRenderer::GameMapDebugRenderer(GameMap* map)
	: m_map(map)
	, m_drawSuperChunkBounds(true)
	, m_drawChunkBounds(true)
	, m_drawActiveOnly(false)
	, m_highlightPlayerLocation(true)
{
}

GameMapDebugRenderer::~GameMapDebugRenderer()
{
}

void GameMapDebugRenderer::Render(Camera const& camera) const
{
	if (!m_map) return;

	if (m_drawGeneratedPixels) {
		RenderGeneratedPixels(camera);
	}

	if (m_drawSuperChunkBounds) {
		RenderSuperChunkBounds(camera);
	}

	if (m_drawChunkBounds) {
		RenderChunkBounds(camera);
	}

	if (m_highlightPlayerLocation) {
		RenderPlayerLocationHighlight(camera);
	}

	if (m_drawPixelGrid) {
		RenderPixelGrid(camera);
	}

	if (m_drawFrustumCulling) {
		RenderFrustumCulling(camera);
	}
}

void GameMapDebugRenderer::RenderSuperChunkBounds(Camera const& camera) const
{
	UNUSED(camera);

	std::vector<Vertex_PCU> verts;
	IntVec2 gridSize = m_map->GetSuperChunkGridSize();

	for (int y = 0; y < gridSize.y; ++y) {
		for (int x = 0; x < gridSize.x; ++x) {
			SuperChunk* sc = m_map->GetSuperChunkByCoords(IntVec2(x, y));
			if (!sc) continue;

			// Skip inactive if flag is set
			if (m_drawActiveOnly && !sc->IsActive()) {
				continue;
			}

			AABB2 bounds = sc->GetWorldBounds();
			Rgba8 color = sc->IsActive() ? Rgba8(0, 255, 0, 255) : Rgba8(128, 128, 128, 128);

			// Draw wireframe
			AddVertsForAABBWire2D(verts, bounds, color, 3.0f, false);

			// Draw corner markers
			float markerSize = 10.0f;
			Rgba8 markerColor = sc->IsActive() ? Rgba8(255, 255, 0, 255) : Rgba8(200, 200, 200, 128);

			// Bottom-left corner
			AABB2 marker(
				bounds.m_mins.x,
				bounds.m_mins.y,
				bounds.m_mins.x + markerSize,
				bounds.m_mins.y + markerSize
			);
			AddVertsForAABB2D(verts, marker, markerColor);
		}
	}

	if (!verts.empty()) {
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->DrawVertexArray(verts);
	}
}

void GameMapDebugRenderer::RenderChunkBounds(Camera const& camera) const
{
	UNUSED(camera);

	std::vector<Vertex_PCU> verts;
	IntVec2 gridSize = m_map->GetSuperChunkGridSize();

	for (int sy = 0; sy < gridSize.y; ++sy) {
		for (int sx = 0; sx < gridSize.x; ++sx) {
			SuperChunk* sc = m_map->GetSuperChunkByCoords(IntVec2(sx, sy));
			if (!sc) continue;

			// Skip inactive if flag is set
			if (m_drawActiveOnly && !sc->IsActive()) {
				continue;
			}

			// Draw all chunks within this super chunk
			for (int cy = 0; cy < CHUNKS_PER_SUPER_CHUNK; ++cy) {
				for (int cx = 0; cx < CHUNKS_PER_SUPER_CHUNK; ++cx) {
					CellChunk* chunk = sc->GetChunk(cx, cy);
					if (!chunk) continue;

					AABB2 bounds = chunk->GetWorldBounds();
					Rgba8 color = Rgba8(100, 150, 255, 128);

					// Draw thin wireframe
					AddVertsForAABBWire2D(verts, bounds, color, 1.0f, false);
				}
			}
		}
	}

	if (!verts.empty()) {
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->DrawVertexArray(verts);
	}
}

void GameMapDebugRenderer::RenderPlayerLocationHighlight(Camera const& camera) const
{
	UNUSED(camera);

	std::vector<Vertex_PCU> verts;

	// Get player's current chunk and super chunk
	CellChunk* playerChunk = m_map->GetPlayerCurrentChunk();
	SuperChunk* playerSuperChunk = m_map->GetPlayerCurrentSuperChunk();

	// Highlight player's super chunk with bright cyan
	if (playerSuperChunk) {
		AABB2 scBounds = playerSuperChunk->GetWorldBounds();
		Rgba8 scColor(0, 255, 255, 180); // Bright cyan with transparency

		// Draw thick wireframe
		AddVertsForAABBWire2D(verts, scBounds, scColor, 5.0f, false);

		// Draw corner markers
		float markerSize = 20.0f;
		Rgba8 markerColor(0, 255, 255, 255); // Solid cyan

		// Bottom-left corner
		AABB2 blMarker(
			scBounds.m_mins.x,
			scBounds.m_mins.y,
			scBounds.m_mins.x + markerSize,
			scBounds.m_mins.y + markerSize
		);
		AddVertsForAABB2D(verts, blMarker, markerColor);

		// Top-right corner
		AABB2 trMarker(
			scBounds.m_maxs.x - markerSize,
			scBounds.m_maxs.y - markerSize,
			scBounds.m_maxs.x,
			scBounds.m_maxs.y
		);
		AddVertsForAABB2D(verts, trMarker, markerColor);
	}

	// Highlight player's chunk with bright magenta
	if (playerChunk) {
		AABB2 chunkBounds = playerChunk->GetWorldBounds();
		Rgba8 chunkColor(255, 0, 255, 220); // Bright magenta with transparency

		// Draw very thick wireframe
		AddVertsForAABBWire2D(verts, chunkBounds, chunkColor, 4.0f, false);

		// Draw a small filled square at chunk center
		Vec2 chunkCenter = chunkBounds.GetCenter();
		float centerMarkerSize = 8.0f;
		AABB2 centerMarker(
			chunkCenter.x - centerMarkerSize,
			chunkCenter.y - centerMarkerSize,
			chunkCenter.x + centerMarkerSize,
			chunkCenter.y + centerMarkerSize
		);
		AddVertsForAABB2D(verts, centerMarker, Rgba8(255, 0, 255, 255)); // Solid magenta
	}

	if (!verts.empty()) {
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->DrawVertexArray(verts);
	}
}

void GameMapDebugRenderer::RenderGeneratedPixels(Camera const& camera) const
{
	// 获取RegionManager（需要GameMap提供访问接口）
	RegionManager* regionManager = m_map->GetRegionManager();
	if (!regionManager) return;

	std::vector<Vertex_PCU> pixelVerts;

	// ⭐ 遍历所有已生成的Region
	const std::map<RegionBounds, std::unique_ptr<RegionGenerationData>>& regions =
		regionManager->GetAllRegions();

	for (auto const& [regionBound, regionData] : regions)
	{
		if (!regionData) continue;

		// 遍历该Region覆盖的所有SuperChunk
		for (int scY = regionBound.m_bottomLeftSC.y; scY <= regionBound.m_topRightSC.y; ++scY)
		{
			for (int scX = regionBound.m_bottomLeftSC.x; scX <= regionBound.m_topRightSC.x; ++scX)
			{
				IntVec2 scCoords(scX, scY);

				// 遍历SuperChunk内的所有chunk
				for (int localY = 0; localY < CHUNKS_PER_SUPER_CHUNK; ++localY)
				{
					for (int localX = 0; localX < CHUNKS_PER_SUPER_CHUNK; ++localX)
					{
						IntVec2 globalChunkCoords(
							scCoords.x * CHUNKS_PER_SUPER_CHUNK + localX,
							scCoords.y * CHUNKS_PER_SUPER_CHUNK + localY
						);

						// 绘制该chunk的所有herringbone pixels
						for (int hpixelY = 0; hpixelY < HB_HPIXELS_PER_CHUNK; ++hpixelY)
						{
							for (int hpixelX = 0; hpixelX < HB_HPIXELS_PER_CHUNK; ++hpixelX)
							{
								IntVec2 localCell(
									hpixelX * HB_CELLS_PER_HPIXEL,
									hpixelY * HB_CELLS_PER_HPIXEL
								);

								Rgba8 pixelColor = regionData->GetPixelForChunk(
									globalChunkCoords, localCell
								);

								int worldX = globalChunkCoords.x * CHUNK_SIZE + localCell.x;
								int worldY = globalChunkCoords.y * CHUNK_SIZE + localCell.y;

								Rgba8 drawColor = pixelColor;

								Vec2 mins((float)worldX, (float)worldY);
								Vec2 maxs((float)(worldX + HB_CELLS_PER_HPIXEL),
									(float)(worldY + HB_CELLS_PER_HPIXEL));
								AABB2 pixelBox(mins, maxs);

								AddVertsForAABB2D(pixelVerts, pixelBox, drawColor);
							}
						}
					}
				}
			}
		}
	}

	if (!pixelVerts.empty()) {
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->DrawVertexArray(pixelVerts);
	}
}

void GameMapDebugRenderer::RenderPixelGrid(Camera const& camera) const
{
	RegionManager* regionManager = m_map->GetRegionManager();
	if (!regionManager) return;

	std::vector<Vertex_PCU> verts;

	// 遍历所有已生成的Region
	const std::map<RegionBounds, std::unique_ptr<RegionGenerationData>>& regions =
		regionManager->GetAllRegions();

	for (auto const& [regionBound, regionData] : regions)
	{
		if (!regionData) continue;

		// 获取该Region的所有tile placements
		const std::vector<HbTilePlacement>& tilePlacements = regionData->GetTilePlacements();

		for (HbTilePlacement const& placement : tilePlacements)
		{
			
			if (!placement.m_selectedTile)
			{
				continue;
			}

			// 转换为 cell 坐标（世界坐标）
			float worldX = placement.m_bottomLeftHPixel.x * HB_CELLS_PER_HPIXEL;
			float worldY = placement.m_bottomLeftHPixel.y * HB_CELLS_PER_HPIXEL;
			float worldWidth = placement.m_sizeHPixel.x * HB_CELLS_PER_HPIXEL;
			float worldHeight = placement.m_sizeHPixel.y * HB_CELLS_PER_HPIXEL;

			// 创建矩形边界
			AABB2 bounds(worldX, worldY, worldX + worldWidth, worldY + worldHeight);

			// 横向 tile = 蓝色，纵向 tile = 红色
			Rgba8 wireColor;
			if (placement.m_orientation == HbTileOrientation::HORIZONTAL)
			{
				wireColor = Rgba8(0, 100, 255, 255);  // 蓝色
			}
			else
			{
				wireColor = Rgba8(255, 0, 0, 255);     // 红色
			}

			// 添加边框
			AddVertsForAABBWire2D(verts, bounds, wireColor, 4.f, false);

			// === 添加约束标记（在框内） ===
			float markerSize = 4.0f;   // 标记方块的大小
			float inset = 4.0f;        // 向内偏移距离，确保在框内

			if (placement.m_orientation == HbTileOrientation::HORIZONTAL)
			{
				// 横向 tile 的 6 条边
				// 边 0: TOP_LEFT (顶部左侧)
				HbEdgeConstraint edge0 = placement.m_selectedTile->GetEdgeConstraint(0);
				Vec2 pos0(worldX + worldWidth * 0.25f, worldY + worldHeight - inset);
				AABB2 marker0(pos0.x - markerSize, pos0.y - markerSize,
					pos0.x + markerSize, pos0.y + markerSize);
				AddVertsForAABB2D(verts, marker0, edge0.m_color);

				// 边 1: TOP_RIGHT (顶部右侧)
				HbEdgeConstraint edge1 = placement.m_selectedTile->GetEdgeConstraint(1);
				Vec2 pos1(worldX + worldWidth * 0.75f, worldY + worldHeight - inset);
				AABB2 marker1(pos1.x - markerSize, pos1.y - markerSize,
					pos1.x + markerSize, pos1.y + markerSize);
				AddVertsForAABB2D(verts, marker1, edge1.m_color);

				// 边 2: RIGHT (右侧中间)
				HbEdgeConstraint edge2 = placement.m_selectedTile->GetEdgeConstraint(2);
				Vec2 pos2(worldX + worldWidth - inset, worldY + worldHeight * 0.5f);
				AABB2 marker2(pos2.x - markerSize, pos2.y - markerSize,
					pos2.x + markerSize, pos2.y + markerSize);
				AddVertsForAABB2D(verts, marker2, edge2.m_color);

				// 边 3: BOTTOM_RIGHT (底部右侧)
				HbEdgeConstraint edge3 = placement.m_selectedTile->GetEdgeConstraint(3);
				Vec2 pos3(worldX + worldWidth * 0.75f, worldY + inset);
				AABB2 marker3(pos3.x - markerSize, pos3.y - markerSize,
					pos3.x + markerSize, pos3.y + markerSize);
				AddVertsForAABB2D(verts, marker3, edge3.m_color);

				// 边 4: BOTTOM_LEFT (底部左侧)
				HbEdgeConstraint edge4 = placement.m_selectedTile->GetEdgeConstraint(4);
				Vec2 pos4(worldX + worldWidth * 0.25f, worldY + inset);
				AABB2 marker4(pos4.x - markerSize, pos4.y - markerSize,
					pos4.x + markerSize, pos4.y + markerSize);
				AddVertsForAABB2D(verts, marker4, edge4.m_color);

				// 边 5: LEFT (左侧中间)
				HbEdgeConstraint edge5 = placement.m_selectedTile->GetEdgeConstraint(5);
				Vec2 pos5(worldX + inset, worldY + worldHeight * 0.5f);
				AABB2 marker5(pos5.x - markerSize, pos5.y - markerSize,
					pos5.x + markerSize, pos5.y + markerSize);
				AddVertsForAABB2D(verts, marker5, edge5.m_color);
			}
			else // VERTICAL
			{
				// 纵向 tile 的 6 条边
				// 边 0: TOP (顶部中间)
				HbEdgeConstraint edge0 = placement.m_selectedTile->GetEdgeConstraint(0);
				Vec2 pos0(worldX + worldWidth * 0.5f, worldY + worldHeight - inset);
				AABB2 marker0(pos0.x - markerSize, pos0.y - markerSize,
					pos0.x + markerSize, pos0.y + markerSize);
				AddVertsForAABB2D(verts, marker0, edge0.m_color);

				// 边 1: RIGHT_TOP (右侧上部)
				HbEdgeConstraint edge1 = placement.m_selectedTile->GetEdgeConstraint(1);
				Vec2 pos1(worldX + worldWidth - inset, worldY + worldHeight * 0.75f);
				AABB2 marker1(pos1.x - markerSize, pos1.y - markerSize,
					pos1.x + markerSize, pos1.y + markerSize);
				AddVertsForAABB2D(verts, marker1, edge1.m_color);

				// 边 2: RIGHT_BOTTOM (右侧下部)
				HbEdgeConstraint edge2 = placement.m_selectedTile->GetEdgeConstraint(2);
				Vec2 pos2(worldX + worldWidth - inset, worldY + worldHeight * 0.25f);
				AABB2 marker2(pos2.x - markerSize, pos2.y - markerSize,
					pos2.x + markerSize, pos2.y + markerSize);
				AddVertsForAABB2D(verts, marker2, edge2.m_color);

				// 边 3: BOTTOM (底部中间)
				HbEdgeConstraint edge3 = placement.m_selectedTile->GetEdgeConstraint(3);
				Vec2 pos3(worldX + worldWidth * 0.5f, worldY + inset);
				AABB2 marker3(pos3.x - markerSize, pos3.y - markerSize,
					pos3.x + markerSize, pos3.y + markerSize);
				AddVertsForAABB2D(verts, marker3, edge3.m_color);

				// 边 4: LEFT_BOTTOM (左侧下部)
				HbEdgeConstraint edge4 = placement.m_selectedTile->GetEdgeConstraint(4);
				Vec2 pos4(worldX + inset, worldY + worldHeight * 0.25f);
				AABB2 marker4(pos4.x - markerSize, pos4.y - markerSize,
					pos4.x + markerSize, pos4.y + markerSize);
				AddVertsForAABB2D(verts, marker4, edge4.m_color);

				// 边 5: LEFT_TOP (左侧上部)
				HbEdgeConstraint edge5 = placement.m_selectedTile->GetEdgeConstraint(5);
				Vec2 pos5(worldX + inset, worldY + worldHeight * 0.75f);
				AABB2 marker5(pos5.x - markerSize, pos5.y - markerSize,
					pos5.x + markerSize, pos5.y + markerSize);
				AddVertsForAABB2D(verts, marker5, edge5.m_color);
			}
		}
	}
	// 渲染所有顶点
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray(verts);

	//-----------------
	//if (m_tilePlacements.empty())
	//{
	//	return;
	//}
	//std::vector<Vertex_PCU> verts;
	//	
	//// 遍历所有已放置的 tile
	//for (const HbTilePlacement& placement : m_tilePlacements)
	//{
	//	if (!placement.m_selectedTile)
	//	{
	//		continue;
	//	}
	//	
	//	// 转换为 cell 坐标（世界坐标）
	//	float worldX = placement.m_bottomLeftHPixel.x * 8.0f;
	//	float worldY = placement.m_bottomLeftHPixel.y * 8.0f;
	//	float worldWidth = placement.m_sizeHPixel.x * 8.0f;
	//	float worldHeight = placement.m_sizeHPixel.y * 8.0f;
	//	
	//	// 创建矩形边界
	//	AABB2 bounds(worldX, worldY, worldX + worldWidth, worldY + worldHeight);
	//	
	//	// 横向 tile = 蓝色，纵向 tile = 红色
	//	Rgba8 wireColor;
	//	if (placement.m_orientation == HbTileOrientation::HORIZONTAL)
	//	{
	//		wireColor = Rgba8(0, 100, 255, 255);  // 蓝色
	//	}
	//	else
	//	{
	//		wireColor = Rgba8(255, 0, 0, 255);     // 红色
	//	}
	//	
	//	// 添加边框
	//	AddVertsForAABBWire2D(verts, bounds, wireColor, 2.f, false);
	//	
	//	// === 添加约束标记（在框内） ===
	//	float markerSize = 2.0f;   // 标记方块的大小
	//	float inset = 4.0f;        // 向内偏移距离，确保在框内
	//	
	//	if (placement.m_orientation == HbTileOrientation::HORIZONTAL)
	//	{
	//		// 横向 tile 的 6 条边
	//		// 边 0: TOP_LEFT (顶部左侧)
	//		HbEdgeConstraint edge0 = placement.m_selectedTile->GetEdgeConstraint(0);
	//		Vec2 pos0(worldX + worldWidth * 0.25f, worldY + worldHeight - inset);
	//		AABB2 marker0(pos0.x - markerSize, pos0.y - markerSize,
	//			pos0.x + markerSize, pos0.y + markerSize);
	//		AddVertsForAABB2D(verts, marker0, edge0.m_color);
	//	
	//		// 边 1: TOP_RIGHT (顶部右侧)
	//		HbEdgeConstraint edge1 = placement.m_selectedTile->GetEdgeConstraint(1);
	//		Vec2 pos1(worldX + worldWidth * 0.75f, worldY + worldHeight - inset);
	//		AABB2 marker1(pos1.x - markerSize, pos1.y - markerSize,
	//			pos1.x + markerSize, pos1.y + markerSize);
	//		AddVertsForAABB2D(verts, marker1, edge1.m_color);
	//	
	//		// 边 2: RIGHT (右侧中间)
	//		HbEdgeConstraint edge2 = placement.m_selectedTile->GetEdgeConstraint(2);
	//		Vec2 pos2(worldX + worldWidth - inset, worldY + worldHeight * 0.5f);
	//		AABB2 marker2(pos2.x - markerSize, pos2.y - markerSize,
	//			pos2.x + markerSize, pos2.y + markerSize);
	//		AddVertsForAABB2D(verts, marker2, edge2.m_color);
	//	
	//		// 边 3: BOTTOM_RIGHT (底部右侧)
	//		HbEdgeConstraint edge3 = placement.m_selectedTile->GetEdgeConstraint(3);
	//		Vec2 pos3(worldX + worldWidth * 0.75f, worldY + inset);
	//		AABB2 marker3(pos3.x - markerSize, pos3.y - markerSize,
	//			pos3.x + markerSize, pos3.y + markerSize);
	//		AddVertsForAABB2D(verts, marker3, edge3.m_color);
	//	
	//		// 边 4: BOTTOM_LEFT (底部左侧)
	//		HbEdgeConstraint edge4 = placement.m_selectedTile->GetEdgeConstraint(4);
	//		Vec2 pos4(worldX + worldWidth * 0.25f, worldY + inset);
	//		AABB2 marker4(pos4.x - markerSize, pos4.y - markerSize,
	//			pos4.x + markerSize, pos4.y + markerSize);
	//		AddVertsForAABB2D(verts, marker4, edge4.m_color);
	//	
	//		// 边 5: LEFT (左侧中间)
	//		HbEdgeConstraint edge5 = placement.m_selectedTile->GetEdgeConstraint(5);
	//		Vec2 pos5(worldX + inset, worldY + worldHeight * 0.5f);
	//		AABB2 marker5(pos5.x - markerSize, pos5.y - markerSize,
	//			pos5.x + markerSize, pos5.y + markerSize);
	//		AddVertsForAABB2D(verts, marker5, edge5.m_color);
	//	}
	//	else // VERTICAL
	//	{
	//		// 纵向 tile 的 6 条边
	//		// 边 0: TOP (顶部中间)
	//		HbEdgeConstraint edge0 = placement.m_selectedTile->GetEdgeConstraint(0);
	//		Vec2 pos0(worldX + worldWidth * 0.5f, worldY + worldHeight - inset);
	//		AABB2 marker0(pos0.x - markerSize, pos0.y - markerSize,
	//			pos0.x + markerSize, pos0.y + markerSize);
	//		AddVertsForAABB2D(verts, marker0, edge0.m_color);
	//	
	//		// 边 1: RIGHT_TOP (右侧上部)
	//		HbEdgeConstraint edge1 = placement.m_selectedTile->GetEdgeConstraint(1);
	//		Vec2 pos1(worldX + worldWidth - inset, worldY + worldHeight * 0.75f);
	//		AABB2 marker1(pos1.x - markerSize, pos1.y - markerSize,
	//			pos1.x + markerSize, pos1.y + markerSize);
	//		AddVertsForAABB2D(verts, marker1, edge1.m_color);
	//	
	//		// 边 2: RIGHT_BOTTOM (右侧下部)
	//		HbEdgeConstraint edge2 = placement.m_selectedTile->GetEdgeConstraint(2);
	//		Vec2 pos2(worldX + worldWidth - inset, worldY + worldHeight * 0.25f);
	//		AABB2 marker2(pos2.x - markerSize, pos2.y - markerSize,
	//			pos2.x + markerSize, pos2.y + markerSize);
	//		AddVertsForAABB2D(verts, marker2, edge2.m_color);
	//	
	//		// 边 3: BOTTOM (底部中间)
	//		HbEdgeConstraint edge3 = placement.m_selectedTile->GetEdgeConstraint(3);
	//		Vec2 pos3(worldX + worldWidth * 0.5f, worldY + inset);
	//		AABB2 marker3(pos3.x - markerSize, pos3.y - markerSize,
	//			pos3.x + markerSize, pos3.y + markerSize);
	//		AddVertsForAABB2D(verts, marker3, edge3.m_color);
	//	
	//		// 边 4: LEFT_BOTTOM (左侧下部)
	//		HbEdgeConstraint edge4 = placement.m_selectedTile->GetEdgeConstraint(4);
	//		Vec2 pos4(worldX + inset, worldY + worldHeight * 0.25f);
	//		AABB2 marker4(pos4.x - markerSize, pos4.y - markerSize,
	//			pos4.x + markerSize, pos4.y + markerSize);
	//		AddVertsForAABB2D(verts, marker4, edge4.m_color);
	//	
	//		// 边 5: LEFT_TOP (左侧上部)
	//		HbEdgeConstraint edge5 = placement.m_selectedTile->GetEdgeConstraint(5);
	//		Vec2 pos5(worldX + inset, worldY + worldHeight * 0.75f);
	//		AABB2 marker5(pos5.x - markerSize, pos5.y - markerSize,
	//			pos5.x + markerSize, pos5.y + markerSize);
	//		AddVertsForAABB2D(verts, marker5, edge5.m_color);
	//	}
	//}
	//	
	//// 渲染所有顶点
	//g_theRenderer->BindTexture(nullptr);
	//g_theRenderer->SetModelConstants();
	//g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	//g_theRenderer->DrawVertexArray(verts);
}

void GameMapDebugRenderer::RenderFrustumCulling(Camera const& camera) const
{
	if (!m_map) return;

	std::vector<Vertex_PCU> verts;
	IntVec2 gridSize = m_map->GetSuperChunkGridSize();

	// 遍历所有 SuperChunk
	for (int sy = 0; sy < gridSize.y; ++sy) 
	{
		for (int sx = 0; sx < gridSize.x; ++sx) 
		{
			SuperChunk* sc = m_map->GetSuperChunkByCoords(IntVec2(sx, sy));
			if (!sc || !sc->IsActive()) continue;

			// 如果 SuperChunk 可见，在左上角画蓝色方块
			if (sc->GetIsVisible()) 
			{
				AABB2 scBounds = sc->GetWorldBounds();
				float markerSize = 20.0f;
				AABB2 marker(
					scBounds.m_mins.x,                 // 左上角 x (左边)
					scBounds.m_maxs.y - markerSize,    // 左上角 y (上边往下)
					scBounds.m_mins.x + markerSize,    // 
					scBounds.m_maxs.y                  // 
				);
				AddVertsForAABB2D(verts, marker, Rgba8::BLUE);

				for (int cy = 0; cy < CHUNKS_PER_SUPER_CHUNK; ++cy) 
				{
					for (int cx = 0; cx < CHUNKS_PER_SUPER_CHUNK; ++cx) 
					{
						CellChunk* chunk = sc->GetChunk(cx, cy);
						if (!chunk) continue;

						// 如果 Chunk 可见，在左上角画红色方块
						if (chunk->GetIsVisible()) {
							AABB2 chunkBounds = chunk->GetWorldBounds();
							float markerSize = 5.0f;
							AABB2 marker(
								chunkBounds.m_mins.x,                  // 左上角 x (左边)
								chunkBounds.m_maxs.y - markerSize,     // 左上角 y (上边往下)
								chunkBounds.m_mins.x + markerSize,     // 
								chunkBounds.m_maxs.y                   // 
							);
							AddVertsForAABB2D(verts, marker, Rgba8::BLUE);
						}
					}
				}
			}
		}
	}

	if (!verts.empty()) {
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->DrawVertexArray(verts);
	}
}

//void GameMapDebugRenderer::RenderPixelGrid(Camera const& camera) const
//{
//	std::vector<Vertex_PCU> gridVerts;
//	Rgba8 gridColor(255, 255, 0, 64);  // 黄色半透明
//
//	// 获取相机视野范围（优化：只绘制可见区域）
//	AABB2 cameraBounds = camera.GetCameraBounds();
//
//	int minChunkX = (int)cameraBounds.m_mins.x / CHUNK_SIZE - 1;
//	int maxChunkX = (int)cameraBounds.m_maxs.x / CHUNK_SIZE + 1;
//	int minChunkY = (int)cameraBounds.m_mins.y / CHUNK_SIZE - 1;
//	int maxChunkY = (int)cameraBounds.m_maxs.y / CHUNK_SIZE + 1;
//
//	// 绘制herringbone pixel网格线
//	for (int chunkY = minChunkY; chunkY <= maxChunkY; ++chunkY) {
//		for (int chunkX = minChunkX; chunkX <= maxChunkX; ++chunkX) {
//			int baseWorldX = chunkX * CHUNK_SIZE;
//			int baseWorldY = chunkY * CHUNK_SIZE;
//
//			// 绘制chunk内的pixel网格
//			for (int i = 0; i <= HB_HPIXELS_PER_CHUNK; ++i) {
//				int offset = i * HB_CELLS_PER_HPIXEL;
//
//				// 垂直线
//				Vec2 vStart((float)(baseWorldX + offset), (float)baseWorldY);
//				Vec2 vEnd((float)(baseWorldX + offset), (float)(baseWorldY + CHUNK_SIZE));
//				AddVertsForLineSegment2D(gridVerts, vStart, vEnd, 0.2f, gridColor);
//
//				// 水平线
//				Vec2 hStart((float)baseWorldX, (float)(baseWorldY + offset));
//				Vec2 hEnd((float)(baseWorldX + CHUNK_SIZE), (float)(baseWorldY + offset));
//				AddVertsForLineSegment2D(gridVerts, hStart, hEnd, 0.2f, gridColor);
//			}
//		}
//	}
//
//	if (!gridVerts.empty()) {
//		g_theRenderer->BindTexture(nullptr);
//		g_theRenderer->DrawVertexArray(gridVerts);
//	}
//}