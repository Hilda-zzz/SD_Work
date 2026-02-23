// GameMapDebugRenderer.cpp
#include "GameMapDebugRenderer.hpp"
#include "GameMap.hpp"
#include "SuperChunk.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/RegionManager.hpp"
#include "ThirdParty/ImGui/imgui.h"
#include "GamePlayer.hpp"
#include "CellMatBrush.hpp"

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

	if (m_drawActiveChunkBound) {
		RenderActiveChunkBounds();
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

void GameMapDebugRenderer::RenderActiveChunkBounds() const
{
	std::vector<Vertex_PCU> verts;
	IntVec2 gridSize = m_map->GetSuperChunkGridSize();

	for (int sy = 0; sy < gridSize.y; ++sy) 
	{
		for (int sx = 0; sx < gridSize.x; ++sx) 
		{
			SuperChunk* sc = m_map->GetSuperChunkByCoords(IntVec2(sx, sy));
			if (!sc) continue;

			// Skip inactive if flag is set
			if (m_drawActiveOnly && !sc->IsActive()) 
			{
				continue;
			}

			// Draw all chunks within this super chunk
			for (int cy = 0; cy < CHUNKS_PER_SUPER_CHUNK; ++cy)
			{
				for (int cx = 0; cx < CHUNKS_PER_SUPER_CHUNK; ++cx) 
				{
					CellChunk* chunk = sc->GetChunk(cx, cy);
					if (!chunk||!chunk->IsDirty()) continue;

					AABB2 bounds = chunk->GetWorldBounds();
					Rgba8 color = Rgba8::CYAN;

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

void GameMapDebugRenderer::RenderMaterialSelector(GamePlayer* player)
{
	CellMatType selectedMaterial = player->GetSelectedMaterial();

	// ==== brush set =========
	int brushSize = player->GetBrushSize();
	ImGui::Text("Brush Size: %d", brushSize);

	// Brush size slider
	int newBrushSize = brushSize;
	if (ImGui::SliderInt("##BrushSize", &newBrushSize, 1, 20, "%d")) {
		player->SetBrushSize(newBrushSize);
	}

	// === Current selected material display ===
	const CellMatUIInfo& currentInfo = CellMatManager::s_materialUIInfo[selectedMaterial];
	ImGui::Text("Current: %s", currentInfo.m_name.c_str());
	ImGui::Separator();

	// Two-panel layout: Category buttons (left) + Material buttons (right)
	// Remove all table borders and make it borderless
	if (ImGui::BeginTable("MaterialSelector", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBody)) {
		ImGui::TableSetupColumn("C", ImGuiTableColumnFlags_WidthFixed, 80.0f);
		ImGui::TableSetupColumn("Materials", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		// === Left Panel: Category Selection ===
		//ImGui::Text("Type");
		ImGui::Separator();

		// Category buttons (vertical)
		auto renderCategoryButton = [&](const char* label, PhyType category, bool isToolsCategory = false) {
			bool isSelected = (m_selectedCategory == category && m_showToolsOnly == isToolsCategory);

			ImGui::PushID(label);

			// Set button colors based on state
			if (isSelected) {
				// Selected state - highlighted
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));        // Green
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f)); // Lighter green
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));  // Darker green
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));        // Bright green border
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
			}
			else {
				// Normal state - default gray
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));        // Gray
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Lighter gray
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));  // Darker gray
			}

			bool clicked = ImGui::Button(label, ImVec2(-1, 40));

			// Clean up styles
			if (isSelected) {
				ImGui::PopStyleVar();
				ImGui::PopStyleColor(4);
			}
			else {
				ImGui::PopStyleColor(3);
			}

			ImGui::PopID();
			return clicked;
			};

		// Category buttons with state management
		if (renderCategoryButton("Static\nSolid", PhyType::PHY_STATIC_SOLID)) {
			m_selectedCategory = PhyType::PHY_STATIC_SOLID;
			m_showToolsOnly = false;
		}

		if (renderCategoryButton("Move\nSolid", PhyType::PHY_MOVE_SOLID)) {
			m_selectedCategory = PhyType::PHY_MOVE_SOLID;
			m_showToolsOnly = false;
		}

		if (renderCategoryButton("Liquid", PhyType::PHY_LIQUID)) {
			m_selectedCategory = PhyType::PHY_LIQUID;
			m_showToolsOnly = false;
		}

		if (renderCategoryButton("Pure CA", PhyType::PHY_CELLULAR_AUTOMATON)) {
			m_selectedCategory = PhyType::PHY_CELLULAR_AUTOMATON;
			m_showToolsOnly = false;
		}

		if (renderCategoryButton("Tools", PhyType::PHY_STATIC_SOLID, true)) {
			m_selectedCategory = PhyType::PHY_STATIC_SOLID;
			m_showToolsOnly = true;
		}

		ImGui::TableNextColumn();

		// === Right Panel: Material Selection ===
		//ImGui::Text("Materials");
		ImGui::Separator();

		// Get materials for selected category
		std::vector<CellMatType> materialsToShow;
		if (m_showToolsOnly) {
			materialsToShow = { CellMatType::MAT_EMPTY }; // Only tools
		}
		else {
			// Get materials by physics type
			for (const auto& pair : CellMatManager::s_materialUIInfo) {
				if (pair.second.m_physType == m_selectedCategory && pair.first != CellMatType::MAT_EMPTY) {
					materialsToShow.push_back(pair.first);
				}
			}
		}

		// Render material buttons as squares in a grid layout
		float buttonSize = 35.0f;
		float spacing = 5.0f;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int buttonsPerRow = (int)((panelWidth + spacing) / (buttonSize + spacing));
		buttonsPerRow = std::max(1, buttonsPerRow);

		for (size_t i = 0; i < materialsToShow.size(); ++i) {
			CellMatType matType = materialsToShow[i];
			const CellMatUIInfo& info = CellMatManager::s_materialUIInfo[matType];
			bool isSelected = (selectedMaterial == matType);

			// Start new row if needed
			if (i % buttonsPerRow != 0) {
				ImGui::SameLine();
			}

			ImGui::PushID((int)matType);

			// Set button colors
			ImGui::PushStyleColor(ImGuiCol_Button, info.m_color);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
				ImVec4(info.m_color.x * 1.3f, info.m_color.y * 1.3f, info.m_color.z * 1.3f, info.m_color.w));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive,
				ImVec4(info.m_color.x * 0.8f, info.m_color.y * 0.8f, info.m_color.z * 0.8f, info.m_color.w));

			// Highlight selected material
			if (isSelected) {
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 1.0f, 0.5f, 1.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);
			}

			// Square button with just color (no text)
	/*		if (ImGui::Button("", ImVec2(buttonSize, buttonSize))) {
				player->SetSelectedMaterial(matType);
			}*/

			bool singleClicked = ImGui::Button("", ImVec2(buttonSize, buttonSize));
			bool doubleClicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

			if (singleClicked && !doubleClicked) {
				player->SetSelectedMaterial(matType);
			}
			//else if (doubleClicked) {
			//	player->SetSelectedMaterial(matType);
			//	m_showMaterialProperties = !m_showMaterialProperties;
			//}

			// Clean up styles
			if (isSelected) {
				ImGui::PopStyleVar();
				ImGui::PopStyleColor();
			}
			ImGui::PopStyleColor(3);

			// Show material name on hover
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("%s", info.m_name.c_str());
				ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", info.m_description.c_str());
				ImGui::EndTooltip();
			}

			ImGui::PopID();
		}

		ImGui::EndTable();
	}
}

//void GameMapDebugRenderer::RenderMaterialProperties(CellMatType selectedMaterial)
//{
//	const CellMatDef& matDef = CellMatManager::GetMaterialDef(selectedMaterial);
//	const CellMatUIInfo& uiInfo = CellMatManager::s_materialUIInfo[selectedMaterial];
//
//	// Material header info
//	ImGui::Text("Material: %s", uiInfo.m_name);
//	ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", uiInfo.m_description);
//
//	// Physics type label
//	ImVec4 typeColor;
//	switch (matDef.m_physicsType) {
//	case PhyType::PHY_STATIC_SOLID:
//		typeColor = ImVec4(0.8f, 0.6f, 0.4f, 1.0f);
//		break;
//	case PhyType::PHY_MOVE_SOLID:
//		typeColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
//		break;
//	case PhyType::PHY_LIQUID:
//		typeColor = ImVec4(0.3f, 0.6f, 1.0f, 1.0f);
//		break;
//	}
//
//	ImGui::TextColored(typeColor, "Type: %s", uiInfo.m_physTypeName);
//	ImGui::Separator();
//
//	// Parameter table
//	//RenderParameterTable(matDef);
//}

//-----------------------------------------------------------------------------
// ImGui Debug Control Panel
//-----------------------------------------------------------------------------
void GameMapDebugRenderer::RenderDebugUI()
{
	// Note: Make sure ImGui context is initialized before calling this
	// This is typically called in your Game::Update() or similar location

	ImGui::Begin("GameMap Debug Renderer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Debug Visualization Controls");
	ImGui::Separator();

	// Super Chunk Bounds Section
	ImGui::Text("Super Chunks:");
	ImGui::Checkbox("Draw Super Chunk Bounds", &m_drawSuperChunkBounds);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Show green wireframes around super chunks\nActive: Green, Inactive: Gray");
	}

	// Chunk Bounds Section
	ImGui::Checkbox("Draw Chunk Bounds", &m_drawChunkBounds);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Show blue wireframes around individual chunks");
	}

	// Active Only Filter
	ImGui::Checkbox("Active Only", &m_drawActiveOnly);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Only draw bounds for active super chunks/chunks");
	}

	ImGui::Separator();

	// Player Location Highlight
	ImGui::Text("Player:");
	ImGui::Checkbox("Highlight Player Location", &m_highlightPlayerLocation);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Highlight player's current super chunk (Cyan) and chunk (Magenta)");
	}

	ImGui::Separator();

	// Frustum Culling Visualization
	ImGui::Text("Culling:");
	ImGui::Checkbox("Draw Frustum Culling", &m_drawFrustumCulling);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Show blue markers on visible super chunks and chunks\nBlue square in top-left corner = visible");
	}

	ImGui::Separator();

	// Active Chunk
	ImGui::Text("Active Chunk:");
	ImGui::Checkbox("Draw Active Chunk", &m_drawActiveChunkBound);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Show active chunks bound in green line.");
	}

	ImGui::Separator();

	// Generated Pixels Section
	ImGui::Text("Herringbone Generation:");
	ImGui::Checkbox("Draw Generated Pixels", &m_drawGeneratedPixels);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Visualize herringbone tile generation status\nRed: Generated, different shades for variants");
	}

	// Pixel Grid
	ImGui::Checkbox("Draw Pixel Grid", &m_drawPixelGrid);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Show yellow grid lines for herringbone pixels");
	}

	// Pixel Draw Mode (if you have different visualization modes)
	ImGui::Text("Pixel Draw Mode:");
	ImGui::RadioButton("Mode 0: Basic", &m_pixelDrawMode, 0);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Basic pixel visualization");
	}

	ImGui::RadioButton("Mode 1: Detailed", &m_pixelDrawMode, 1);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Detailed pixel information");
	}

	ImGui::RadioButton("Mode 2: Tile Edges", &m_pixelDrawMode, 2);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Show tile edge constraints");
	}

	ImGui::Separator();

	// Quick Toggle Buttons
	ImGui::Text("Quick Actions:");
	if (ImGui::Button("Enable All"))
	{
		m_drawSuperChunkBounds = true;
		m_drawChunkBounds = true;
		m_highlightPlayerLocation = true;
		m_drawFrustumCulling = true;
		m_drawGeneratedPixels = true;
		m_drawPixelGrid = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Disable All"))
	{
		m_drawSuperChunkBounds = false;
		m_drawChunkBounds = false;
		m_highlightPlayerLocation = false;
		m_drawFrustumCulling = false;
		m_drawGeneratedPixels = false;
		m_drawPixelGrid = false;
	}

	// Performance section (optional)
	ImGui::Separator();
	ImGui::Text("Performance:");
	if (m_map)
	{
		IntVec2 gridSize = m_map->GetSuperChunkGridSize();
		ImGui::Text("Super Chunk Grid: %dx%d", gridSize.x, gridSize.y);
		ImGui::Text("Total Super Chunks: %d", gridSize.x * gridSize.y);
	}

	ImGui::End();
}

void GameMapDebugRenderer::RenderMatBrushUI(GamePlayer* player)
{
	// Set window flags
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;

	// Material Selector Window
	if (m_showMaterialSelector) {
		ImGui::SetNextWindowSize(ImVec2(280, 50), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);

		ImGuiWindowFlags window_flags_selector = ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoTitleBar;// |
			//ImGuiWindowFlags_NoBackground; // | ImGuiWindowFlags_NoResize;

		if (ImGui::Begin("Material Brush", &m_showMaterialSelector, window_flags_selector)) {
			RenderMaterialSelector(player);
		}
		ImGui::End();
	}

	// Material Properties Window
	/*if (m_showMaterialProperties) {
		ImGui::SetNextWindowSize(ImVec2(350, 500), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(1250, 20), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Material Properties", &m_showMaterialProperties, window_flags)) {
			RenderMaterialProperties(player->GetSelectedMaterial());
		}
		ImGui::End();
	}*/
}
