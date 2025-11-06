#include "SandboxMap.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "SandboxPlayer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cmath>
#include "ThirdParty/imgui/imgui_internal.h"
#include "Engine/Window/Window.hpp"
#include "CellBehaviorSystem.hpp"
#include "CellMatManager.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "CellBehaviorSystemInChunk.hpp"
#include "Game/RigidbodyManager.hpp"
#include "Game/RigidBodyObject.hpp"
#include "ChunkUpdateJob.hpp"
#include "Engine/JobSystem/JobSystem.hpp"
#include <ThirdParty/box2d/include/box2d/box2d.h>
#include <Engine/Input/InputSystem.hpp>
#include "ThirdParty/box2d/include/box2d/id.h"
#include "Engine/Math/OBB2.hpp"
#include "Game.hpp"
#include "Engine/Math/IntRange.hpp"
extern Renderer* g_theRenderer;
extern Window* g_theWindow;
extern JobSystem* g_theJobSystem;
extern InputSystem* g_theInput;

SandboxMap::SandboxMap(SandboxPlayer* playerPtr, IntVec2 const& size) : BaseMap(size, 64)
{
	m_player = playerPtr;
	m_mapBounds = AABB2(0.f, 0.f, (float)size.x, (float)size.y);
	// === NEW: 计算需要多少个chunk ===
	m_chunkGridSize.x = (m_mapSize.x + CHUNK_SIZE - 1) / CHUNK_SIZE;  // 向上取整
	m_chunkGridSize.y = (m_mapSize.y + CHUNK_SIZE - 1) / CHUNK_SIZE;

	// === NEW: 创建chunk网格 ===
	m_chunks.resize(m_chunkGridSize.y);
	for (int chunkY = 0; chunkY < m_chunkGridSize.y; ++chunkY) {
		m_chunks[chunkY].resize(m_chunkGridSize.x);
		for (int chunkX = 0; chunkX < m_chunkGridSize.x; ++chunkX) {
			m_chunks[chunkY][chunkX] = new CellChunk(IntVec2(chunkX, chunkY),this);
		}
	}

	// === OLD: 保留用于对比测试 ===
	// m_grid = std::vector(size.y, std::vector(size.x, Cell()));

	m_player->SetCurMap(this);
	Initialize();


	//================
	CreateBox2dWorld();
	
	m_rigidBodyManager = new RigidBodyManager(this,m_b2WorldId);

	CreateTestGround();
}

SandboxMap::~SandboxMap()
{
	for (auto& row : m_chunks) {
		for (auto& chunk : row) {
			delete chunk;
			chunk = nullptr;
		}
	}
	m_chunks.clear();
	delete m_rigidBodyManager;
}

void SandboxMap::Update(float deltaTime)
{
	m_deltaTime = deltaTime;
	UpdateMouseGridPosition();

	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE)) {
		// 在屏幕中心上方生成方块
		Vec2 spawnPos = Vec2(
			(float)m_mouseGridX,        // X: 屏幕中心
			(float)m_mouseGridY        // Y: 屏幕上方3/4处
		);
		SpawnTestBox(spawnPos);
	}

	m_player->Update(deltaTime);
	//UpdatePhysics();

	// #TODO: notice the sequence of update()
	// 暂时对于点，全部重新添加
	for (auto& row : m_chunks) {
		for (auto& chunk : row) {
			if (chunk->IsDirty())
				chunk->RebuildVertex();
		}
	}

	UpdateCellsPhysInChunk();

	// Used by debug panel
	UpdateStatistics();

	UpdateCellsChemicalInChunk();

	m_player->RenderImgui();
	RenderDebugDrawPanel();

	if (!B2_IS_NULL(m_b2WorldId)) {
		m_rigidBodyManager->Update(deltaTime);
	}

}

void SandboxMap::Render() const
{
	g_theRenderer->BeginCamera(m_player->m_camera);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray(m_boundVerts);

	// === Chunk Grid Lines (Gray) ===
	if (m_debugSettings.m_drawChunkGrid) {
		std::vector<Vertex_PCU> chunkGridVerts;

		// Vertical lines
		for (int chunkX = 0; chunkX <= m_chunkGridSize.x; ++chunkX) {
			float worldX = static_cast<float>(chunkX * CHUNK_SIZE);
			Vec2 start(worldX, 0.f);
			Vec2 end(worldX, static_cast<float>(m_mapSize.y));
			Rgba8 gridColor(100, 100, 100, 128);
			AddVertsForLinSegment2D(chunkGridVerts, start, end, 0.5f, gridColor);
		}

		// Horizontal lines
		for (int chunkY = 0; chunkY <= m_chunkGridSize.y; ++chunkY) {
			float worldY = static_cast<float>(chunkY * CHUNK_SIZE);
			Vec2 start(0.f, worldY);
			Vec2 end(static_cast<float>(m_mapSize.x), worldY);
			Rgba8 gridColor(100, 100, 100, 128);
			AddVertsForLinSegment2D(chunkGridVerts, start, end, 0.5f, gridColor);
		}

		g_theRenderer->DrawVertexArray(chunkGridVerts);
	}

	// === Static/Dynamic Chunk Borders ===
	if (m_debugSettings.m_drawStaticChunks || m_debugSettings.m_drawDynamicChunks) {
		std::vector<Vertex_PCU> chunkDebugVerts;

		for (auto& row : m_chunks) {
			for (auto& chunk : row) {
				AABB2 chunkBounds = chunk->GetWorldBounds();

				if (chunk->IsDirty() && m_debugSettings.m_drawDynamicChunks) {
					// Green for dynamic (dirty) chunks
					Rgba8 borderColor(0, 255, 0, 200);
					float borderThickness = 1.0f;
					AddVertsForAABBWire2D(chunkDebugVerts, chunkBounds, borderColor, borderThickness,false);
				}
				else if (!chunk->IsDirty() && m_debugSettings.m_drawStaticChunks) {
					// Red for static (non-dirty) chunks
					Rgba8 borderColor(255, 0, 0, 200);
					float borderThickness = 1.0f;
					AddVertsForAABBWire2D(chunkDebugVerts, chunkBounds, borderColor, borderThickness,false);
				}
			}
		}

		g_theRenderer->DrawVertexArray(chunkDebugVerts);
	}

	// === Draw Cells ===
	// std::vector<Vertex_PCU> cellVerts;
	for (auto& row : m_chunks) {
		for (auto& chunk : row) {
			chunk->RenderChunk();
		}
	}
	//g_theRenderer->DrawVertexArray(cellVerts);

	//============== Draw rb =================
	RenderPhysicsDebug();

	for (RigidBodyObject* obj : m_rigidBodyManager->m_testRbList)
	{
		obj->Render();
	}

	m_rigidBodyManager->RenderDebug();
	g_theRenderer->EndCamera(m_player->m_camera);

	RenderImGuiStats();
	RenderCellInfo();
}

void SandboxMap::RenderImGuiStats() const
{
	if (ImGui::Begin("Sandbox Statistics"))
	{
		ImGui::Text("=== Cell Statistics ===");
		ImGui::Text("Total Non-Empty Cells: %d", m_cachedNonEmptyCells);
		ImGui::Text("Total Materials Set: %d", m_totalMaterialsSet);
	}
	ImGui::End();
}
void SandboxMap::RenderCellInfo() const
{
	if (ImGui::Begin("Cell Inspector"))
	{
		ImGui::Text("=== Mouse Position ===");
		ImGui::Text("Grid Position: (%d, %d)", m_mouseGridX, m_mouseGridY);

		// Get chunk information
		IntVec2 chunkIndex = CellChunk::WorldToChunkIndex(m_mouseGridX, m_mouseGridY);
		IntVec2 localCoord = CellChunk::WorldToLocal(m_mouseGridX, m_mouseGridY);

		ImGui::Text("Chunk Index: (%d, %d)", chunkIndex.x, chunkIndex.y);
		ImGui::Text("Local Position: (%d, %d)", localCoord.x, localCoord.y);

		// Get current cell
		Cell currentCell;
		CellChunk* currentChunk = nullptr;
		bool validPosition = false;

		if (IsInBounds(m_mouseGridX, m_mouseGridY))
		{
			currentChunk = const_cast<SandboxMap*>(this)->GetChunkByWorldPos(m_mouseGridX, m_mouseGridY);
			if (currentChunk) {
				currentCell = currentChunk->GetLocalCell(localCoord.x, localCoord.y);
				validPosition = true;
			}
		}

		if (validPosition && currentChunk)
		{
			// Chunk Information
			ImGui::Separator();
			ImGui::Text("=== Chunk Information ===");
			ImGui::Text("Chunk Phase Index: %d", currentChunk->GetPhaseIndex());
			ImGui::Text("Chunk Is Dirty: %s", currentChunk->IsDirty() ? "Yes" : "No");
			ImGui::Text("Active Cells in Chunk: %d", currentChunk->GetActiveCellCount());

			// Cell Information
			ImGui::Separator();
			ImGui::Text("=== Cell Information ===");
			ImGui::Text("Material Type: %s", GetMaterialTypeName(currentCell.m_type));
			ImGui::Text("Is Empty: %s", currentCell.IsEmpty() ? "Yes" : "No");

			// Physics Properties
			ImGui::Separator();
			ImGui::Text("=== Physics Properties ===");
			ImGui::Text("Velocity X: %.3f", currentCell.m_velocityX);
			ImGui::Text("Velocity Y: %.3f", currentCell.m_velocityY);
			ImGui::Text("Accumulated Move X: %.3f", currentCell.m_accumulMoveX);
			ImGui::Text("Accumulated Move Y: %.3f", currentCell.m_accumulMoveY);

			// State Information
			ImGui::Separator();
			ImGui::Text("=== State Information ===");
			ImGui::Text("Updated This Frame: %s", currentCell.m_updatedThisFrame ? "Yes" : "No");
			ImGui::Text("Is Free Falling: %s", currentCell.m_isFreeFalling ? "Yes" : "No");
			ImGui::Text("Frames Without Movement: %d", currentCell.m_framesWithoutMovement);

			if (currentCell.m_type == CellMatType::MAT_WATER ||
				currentCell.m_type == CellMatType::MAT_OIL ||
				currentCell.m_type == CellMatType::MAT_LAVA) {
				ImGui::Text("Liquid Re-collide Times: %d", currentCell.m_liquidReCollideTimes);
			}

			// Visual Properties
			ImGui::Separator();
			ImGui::Text("=== Visual Properties ===");
			ImGui::Text("Color: R=%d G=%d B=%d A=%d",
				currentCell.m_color.r,
				currentCell.m_color.g,
				currentCell.m_color.b,
				currentCell.m_color.a);

			// Color preview
			ImVec4 colorPreview = ImVec4(
				currentCell.m_color.r / 255.0f,
				currentCell.m_color.g / 255.0f,
				currentCell.m_color.b / 255.0f,
				currentCell.m_color.a / 255.0f
			);
			ImGui::ColorEdit4("Color Preview", (float*)&colorPreview,
				ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
		}
		else
		{
			ImGui::Separator();
			ImGui::Text("=== Cell Information ===");
			ImGui::Text("Position out of bounds or invalid");
		}

		// Debug Info
		ImGui::Separator();
		ImGui::Text("=== Debug Info ===");
		Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
		ImGui::Text("Mouse UV: (%.3f, %.3f)", mouseUV.x, mouseUV.y);

		Vec2 mousePosInWorld = AABB2(m_player->m_camera.GetOrthoBottomLeft(),
			m_player->m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);
		ImGui::Text("World Position: (%.3f, %.3f)", mousePosInWorld.x, mousePosInWorld.y);
	}
	ImGui::End();
}

void SandboxMap::RenderDebugDrawPanel()
{
	CellColorMode m_prevMode = m_debugSettings.m_colorMode;
	if (ImGui::Begin("Debug Visualization"))
	{
		// === Grid Visualization Section ===
		ImGui::SeparatorText("Grid Visualization");

		ImGui::Checkbox("Draw Chunk Grid (Gray)", &m_debugSettings.m_drawChunkGrid);
		ImGui::Checkbox("Draw Static Chunks (Red)", &m_debugSettings.m_drawStaticChunks);
		ImGui::Checkbox("Draw Dynamic Chunks (Green)", &m_debugSettings.m_drawDynamicChunks);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// === Cell Coloring Mode Section ===
		ImGui::SeparatorText("Cell Coloring Mode");

		if (ImGui::RadioButton("Normal Colors",
			m_debugSettings.m_colorMode == CellColorMode::NORMAL)) {
			m_debugSettings.m_colorMode = CellColorMode::NORMAL;
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::Text("Display cells with their original material colors");
			ImGui::EndTooltip();
		}

		if (ImGui::RadioButton("Static/Dynamic View",
			m_debugSettings.m_colorMode == CellColorMode::STATIC_DYNAMIC)) {
			m_debugSettings.m_colorMode = CellColorMode::STATIC_DYNAMIC;
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::Text("Red = Static granular particles");
			ImGui::Text("Green = Dynamic falling particles");
			ImGui::Text("White = Static solid blocks");
			ImGui::EndTooltip();
		}

		if (ImGui::RadioButton("Movement Heatmap",
			m_debugSettings.m_colorMode == CellColorMode::FRAME_COUNT)) {
			m_debugSettings.m_colorMode = CellColorMode::FRAME_COUNT;
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::Text("Red->Green gradient based on frames without movement");
			ImGui::Text("Red = Recently moved (0 frames)");
			ImGui::Text("Green = Settled (80+ frames)");
			ImGui::Text("White = Static solid blocks");
			ImGui::EndTooltip();
		}

		ImGui::Spacing();

		// Color legend
		if (m_debugSettings.m_colorMode != CellColorMode::NORMAL) {
			ImGui::Separator();
			ImGui::Text("Color Legend:");

			if (m_debugSettings.m_colorMode == CellColorMode::STATIC_DYNAMIC) {
				ImGui::ColorButton("##red", ImVec4(1, 0, 0, 1),
					ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
				ImGui::SameLine();
				ImGui::Text("Static Particles");

				ImGui::ColorButton("##green", ImVec4(0, 1, 0, 1),
					ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
				ImGui::SameLine();
				ImGui::Text("Dynamic Particles");

				ImGui::ColorButton("##white", ImVec4(1, 1, 1, 1),
					ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
				ImGui::SameLine();
				ImGui::Text("Static Solids");
			}
			else if (m_debugSettings.m_colorMode == CellColorMode::FRAME_COUNT) {
				ImGui::ColorButton("##red", ImVec4(1, 0, 0, 1),
					ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
				ImGui::SameLine();
				ImGui::Text("80+ frames");

				ImGui::ColorButton("##yellow", ImVec4(1, 1, 0, 1),
					ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
				ImGui::SameLine();
				ImGui::Text("40 frames");

				ImGui::ColorButton("##green", ImVec4(0, 1, 0, 1),
					ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
				ImGui::SameLine();
				ImGui::Text("0 frames");

				ImGui::ColorButton("##white", ImVec4(1, 1, 1, 1),
					ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
				ImGui::SameLine();
				ImGui::Text("Static Solids");
			}
		}
		// === rigid body Visualization Section ===
		ImGui::SeparatorText("RigidBody Generation Visualization");

		ImGui::Checkbox("Marching Squares Outline", &m_debugSettings.m_drawMarchingSquares);
		ImGui::Checkbox("Douglas-Peucker Result", &m_debugSettings.m_drawDouglas);
		ImGui::Checkbox("Triangulation (Earclipping)", &m_debugSettings.m_drawTriangleMesh);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
	}
	ImGui::End();

	if (m_debugSettings.m_colorMode != m_prevMode)
	{
		for (auto& row : m_chunks) {
			for (auto& chunk : row) {
				chunk->MarkDirty();
			}
		}
	}
}


Rgba8 SandboxMap::GetCellDebugColor(const Cell& cell, IntVec2 const& worldCoords) const
{
	const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type);

	switch (m_debugSettings.m_colorMode) {
	case CellColorMode::NORMAL:
		return cell.m_color;

	case CellColorMode::STATIC_DYNAMIC:
		// Static solids = White
		if (matDef.m_physicsType == PhyType::PHY_STATIC_SOLID) {
			return Rgba8::WHITE;
		}
		// Dynamic particles = Green, Static particles = Red
		else if (matDef.m_physicsType == PhyType::PHY_MOVE_SOLID ||
			matDef.m_physicsType == PhyType::PHY_LIQUID) {
			return cell.m_isFreeFalling ? Rgba8(0, 255, 0) : Rgba8(255, 0, 0);
		}
		return cell.m_color;

	case CellColorMode::FRAME_COUNT:
		// Static solids = White
		if (matDef.m_physicsType == PhyType::PHY_STATIC_SOLID) {
			return Rgba8::WHITE;
		}
		// Gradient from Red (0 frames) to Green (80+ frames)
		else if (matDef.m_physicsType == PhyType::PHY_MOVE_SOLID ||
			matDef.m_physicsType == PhyType::PHY_LIQUID) {
			float t = GetClamped(cell.m_framesWithoutMovement / 80.0f, 0.0f, 1.0f);
			uint8_t green = static_cast<uint8_t>(255 * (1.0f - t));
			uint8_t red = static_cast<uint8_t>(255 * t);
			return Rgba8(red, green, 0);
		}
		return cell.m_color;

	default:
		return cell.m_color;
	}
}


void SandboxMap::Initialize()
{
	AddVertsForAABBWire2D(m_boundVerts, m_mapBounds, Rgba8::WHITE, m_mapSize.x / 100.f, true);
}


bool SandboxMap::IsValidPosition(int x, int y) const
{
	if (x >= 0 && x < m_mapSize.x && y >= 0 && y < m_mapSize.y) return true;
	return false;
}

Cell& SandboxMap::GetCell(int worldX, int worldY)
{
	IntVec2 chunkIndex = CellChunk::WorldToChunkIndex(worldX, worldY);
	IntVec2 localCoord = CellChunk::WorldToLocal(worldX, worldY);

	CellChunk* chunk = GetChunk(chunkIndex.x, chunkIndex.y);
	GUARANTEE_OR_DIE(chunk != nullptr, "Invalid world position");

	return chunk->GetLocalCell(localCoord.x, localCoord.y);
}

Cell const& SandboxMap::GetCell(int worldX, int worldY) const
{
	IntVec2 chunkIndex = CellChunk::WorldToChunkIndex(worldX, worldY);
	IntVec2 localCoord = CellChunk::WorldToLocal(worldX, worldY);

	CellChunk const* chunk = GetChunk(chunkIndex.x, chunkIndex.y);
	GUARANTEE_OR_DIE(chunk != nullptr, "Invalid world position");

	return chunk->GetLocalCell(localCoord.x, localCoord.y);
}

bool SandboxMap::IsInBounds_Chunk(int worldX, int worldY) const
{
	return worldX >= 0 && worldX < m_mapSize.x &&
		worldY >= 0 && worldY < m_mapSize.y;
}

void SandboxMap::PlaceMaterialInChunk(int worldX, int worldY, CellMatType type, bool isRb)
{
	if (!IsInBounds_Chunk(worldX, worldY)) return;
	Cell& cell = GetCell(worldX, worldY);

	GetChunkByWorldPos(worldX, worldY)->MarkDirty();

	if (cell.m_type == CellMatType::MAT_EMPTY)
	{
		CellMatDef const& curDef = CellMatManager::GetMaterialDef(type);
		if (type == CellMatType::MAT_DYSOLID_FIRE)
		{
			int a = 0;
		}
		m_totalMaterialsSet++;
		cell.m_isBelongRb = isRb;
		cell.m_type = type;
		cell.m_color = curDef.m_color;
		cell.m_lifeCountDown = curDef.m_lifeCountDown.GetRandomInRange(&Game::s_rng);
		cell.m_flameCountDown = curDef.m_flameCountDown.GetRandomInRange(&Game::s_rng);
		cell.m_dissolveCountDown = curDef.m_dissolveCountDowm.GetRandomInRange(&Game::s_rng);
		cell.m_corrosionCountDown = curDef.m_corrosionCountDown.GetRandomInRange(&Game::s_rng);
		if (type == CellMatType::MAT_SAND)
		{
			m_sandSet++;
		}
		if (type == CellMatType::MAT_SALT)
		{
			m_sandSet++;
		}
		else if (type == CellMatType::MAT_WATER)
		{
			m_waterSet++;
		}
		else if (type == CellMatType::MAT_STONE)
		{
			cell.m_isFreeFalling = false;
			m_stoneSet++;
		}
	}
}

void SandboxMap::UpdateChunksOfPhase(int phaseIndex)
{
	if (!m_useJobSystem)
	{
		for (auto& row : m_chunks)
		{
			for (auto& chunk : row) 
			{
				if (chunk->GetPhaseIndex() == phaseIndex)
				{
					if (chunk->IsDirty())
						UpdateSingleChunk(chunk);
					else
						int a = 1;
				}
			}
		}
	}
	else
	{
		std::vector<ChunkUpdateJob*> jobs;
		for (auto& row : m_chunks) 
		{
			for (auto& chunk : row) 
			{
				if (chunk->GetPhaseIndex() == phaseIndex && chunk->IsDirty())
				{
					ChunkUpdateJob* job = new ChunkUpdateJob(chunk, this);
					jobs.push_back(job);
					g_theJobSystem->QueueJob(job);
				}
			}
		}

		int completedCount = 0;
		while (completedCount < (int)jobs.size())
		{
			Job* completedJob = g_theJobSystem->RetrieveCompletedJob();
			if (completedJob != nullptr)
			{
				completedCount++;
				delete completedJob;
			}
			else
			{
				std::this_thread::yield();
			}
		}

		jobs.clear();
	}

}

void SandboxMap::UpdateSingleChunk(CellChunk* chunk)
{
	chunk->ClearDirty();
	IntVec2 chunkIndex = chunk->GetChunkIndex();

	for (int localY = 0; localY < CHUNK_SIZE; ++localY) {
		// 奇数行从左到右,偶数行从右到左
		if (localY % 2 == 0) {
			// 从左到右
			for (int localX = 0; localX < CHUNK_SIZE; localX++) {
				Cell& cell = chunk->GetLocalCell(localX, localY);
				if (cell.IsEmpty()) continue;
				if (cell.m_isBelongRb) continue;
				IntVec2 worldPos = chunk->LocalToWorld(localX, localY);
				CellBehaviorSystemInChunk::UpdateCell(cell, worldPos.x, worldPos.y, this);
			}
		}
		else {
			// 从右到左
			for (int localX = CHUNK_SIZE - 1; localX >= 0; localX--) {
				Cell& cell = chunk->GetLocalCell(localX, localY);
				if (cell.IsEmpty()) continue;
				if (cell.m_isBelongRb) continue;
				IntVec2 worldPos = chunk->LocalToWorld(localX, localY);
				CellBehaviorSystemInChunk::UpdateCell(cell, worldPos.x, worldPos.y, this);
			}
		}
	}
}

bool SandboxMap::MSCanMoveTo(int x, int y, float curDensity) const
{
	if (!IsInBounds(x, y)) {
		return false;
	}

	// ✅ 使用 GetCellInChunk 而不是 m_grid
	Cell const& targetCell = GetCell(x, y);

	// 获取目标cell的密度和物理类型
	const CellMatDef& targetMatDef = CellMatManager::GetMaterialDef(targetCell.m_type);
	float targetDensity = targetMatDef.m_density;

	// MoveSolid 可以移动到的条件：
	// 1. 目标是空的
	// 2. 目标是液体 且 当前密度大于目标密度（浮力）
	return targetCell.IsEmpty() ||
		((curDensity > targetDensity) &&
			targetMatDef.m_physicsType == PhyType::PHY_LIQUID);
}

bool SandboxMap::LiquidCanMoveTo(int x, int y, float curDensity) const
{
	if (!IsInBounds(x, y)) {
		return false;
	}

	Cell const& targetCell = GetCell(x, y);
	float targetDensity = CellMatManager::GetMaterialDef(targetCell.m_type).m_density;
	return curDensity > targetDensity;
}

bool SandboxMap::CanMoveHorizontally(int x, int y, const Cell& cell) const
{
	// 需要有足够的速度 且 有支撑才能水平移动
	return (std::abs(cell.m_velocityX) > 1.0f || std::abs(cell.m_velocityY) > 80.0f) &&
		HasSupport(x, y);
}

bool SandboxMap::HasSupport(int x, int y) const
{
	if (y == 0) {
		return true;
	}

	// ✅ 使用 GetCellInChunk 检查下方是否有非空格子
	return !GetCell(x, y - 1).IsEmpty();
}

void SandboxMap::UpdateStatistics()
{
	// Reset counters
	m_cachedNonEmptyCells = 0;
	m_cachedSandCells = 0;
	m_cachedWaterCells = 0;
	m_cachedStoneCells = 0;

	// Iterate through all chunks
	for (auto& row : m_chunks) {
		for (auto& chunk : row) {
			// Iterate through cells in this chunk
			for (int localY = 0; localY < CHUNK_SIZE; ++localY) {
				for (int localX = 0; localX < CHUNK_SIZE; ++localX) {
					const Cell& cell = chunk->GetLocalCell(localX, localY);

					if (cell.m_type != CellMatType::MAT_EMPTY) {
						m_cachedNonEmptyCells++;

						// Count by material type
						switch (cell.m_type) {
						case CellMatType::MAT_SAND:
							m_cachedSandCells++;
							break;
						case CellMatType::MAT_SALT:
							m_cachedSandCells++; // Salt counted as sand-like
							break;
						case CellMatType::MAT_WATER:
							m_cachedWaterCells++;
							break;
						case CellMatType::MAT_OIL:
							m_cachedWaterCells++; // Oil counted as liquid
							break;
						case CellMatType::MAT_LAVA:
							m_cachedWaterCells++; // Lava counted as liquid
							break;
						case CellMatType::MAT_STONE:
							m_cachedStoneCells++;
							break;
						case CellMatType::MAT_WOOD:
							m_cachedStoneCells++; // Wood counted as solid
							break;
						case CellMatType::MAT_SOIL:
							m_cachedSandCells++; // Soil counted as granular
							break;
						case CellMatType::MAT_GRAVEL:
							m_cachedSandCells++; // Gravel counted as granular
							break;
						default:
							break;
						}
					}
				}
			}
		}
	}
}

void SandboxMap::UpdateMouseGridPosition()
{
	Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
	Vec2 mousePosInWorld = AABB2(m_player->m_camera.GetOrthoBottomLeft(), m_player->m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);
	m_mouseGridX = static_cast<int>(floor(mousePosInWorld.x));
	m_mouseGridY = static_cast<int>(floor(mousePosInWorld.y));
}

float SandboxMap::GetDeltaTime()
{
	return m_deltaTime;
}

void SandboxMap::CreateTestGround()
{
	float groundWidthCells = static_cast<float>(m_mapSize.x);    // 地图宽度
	float groundHeightCells = 32.0f;                          // 高度32 cells = 1 meter
	float groundYCells = groundHeightCells * 0.5f;            // Y位置（在底部）

	// 转换为物理单位（米）
	float groundWidthMeters = groundWidthCells * METERS_PER_CELL;
	float groundHeightMeters = groundHeightCells * METERS_PER_CELL;
	b2Vec2 groundPositionMeters = {
		groundWidthCells * 0.5f * METERS_PER_CELL,  // X: 地图中心
		groundYCells * METERS_PER_CELL               // Y: 底部
	};

	// 创建body定义
	b2BodyDef groundBodyDef = b2DefaultBodyDef();
	groundBodyDef.type = b2_staticBody;                      // 静态body
	groundBodyDef.position = groundPositionMeters;

	// 创建body
	m_testGroundBodyId = b2CreateBody(m_b2WorldId, &groundBodyDef);

	if (B2_IS_NULL(m_testGroundBodyId)) {
		ERROR_AND_DIE("Failed to create test ground body!");
		return;
	}

	// 创建box shape
	b2Polygon groundBox = b2MakeBox(
		groundWidthMeters * 0.5f,   // 半宽
		groundHeightMeters * 0.5f   // 半高
	);

	// 创建shape定义
	b2ShapeDef groundShapeDef = b2DefaultShapeDef();
	groundShapeDef.material.friction = 0.6f;
	groundShapeDef.material.restitution = 0.1f;

	// 附加shape到body
	b2ShapeId groundShapeId = b2CreatePolygonShape(m_testGroundBodyId, &groundShapeDef, &groundBox);

	if (B2_IS_NULL(groundShapeId)) {
		ERROR_AND_DIE("Failed to create test ground shape!");
	}
}

void SandboxMap::SpawnTestBox(Vec2 const& position)
{
	float boxSizeCells = 32.0f;  // 32×32 cells = 1m × 1m

	// 转换为物理单位
	float boxSizeMeters = boxSizeCells * METERS_PER_CELL;  // 1 meter
	b2Vec2 boxPositionMeters = {
		position.x * METERS_PER_CELL,
		position.y * METERS_PER_CELL
	};

	// 创建body定义
	b2BodyDef boxBodyDef = b2DefaultBodyDef();
	boxBodyDef.type = b2_dynamicBody;           // 动态body
	boxBodyDef.position = boxPositionMeters;
	boxBodyDef.linearDamping = 0.1f;
	boxBodyDef.angularDamping = 0.1f;
	boxBodyDef.gravityScale = 1.0f;

	// 创建body
	b2BodyId boxBodyId = b2CreateBody(m_b2WorldId, &boxBodyDef);

	if (B2_IS_NULL(boxBodyId)) {
		ERROR_AND_DIE("Failed to create test box body!");
		return;
	}

	// 创建box shape
	b2Polygon boxShape = b2MakeBox(
		boxSizeMeters * 0.5f,  // 半宽 = 0.5 meter
		boxSizeMeters * 0.5f   // 半高 = 0.5 meter
	);

	// 创建shape定义
	b2ShapeDef boxShapeDef = b2DefaultShapeDef();
	boxShapeDef.density = 1.0f;                  // kg/m²
	boxShapeDef.material.friction = 0.3f;
	boxShapeDef.material.restitution = 0.3f;     // 有一些弹性

	// 附加shape到body
	b2ShapeId boxShapeId = b2CreatePolygonShape(boxBodyId, &boxShapeDef, &boxShape);

	if (B2_IS_NULL(boxShapeId)) {
		ERROR_AND_DIE("Failed to create test box shape!");
		return;
	}

	// 保存body ID
	m_testBoxBodies.push_back(boxBodyId);
}

void SandboxMap::RenderPhysicsDebug() const
{
	if (B2_IS_NULL(m_b2WorldId)) return;

	std::vector<Vertex_PCU> debugVerts;

	// ========== 渲染地面 ==========
	if (!B2_IS_NULL(m_testGroundBodyId)) {
		b2Vec2 groundPosMeters = b2Body_GetPosition(m_testGroundBodyId);
		Vec2 groundPosCells = PhysicsToCell(groundPosMeters);

		// 地面尺寸（元胞单位）
		float groundWidthCells = static_cast<float>(m_mapSize.x);
		float groundHeightCells = 32.0f;

		// 创建AABB2用于渲染
		Vec2 mins = groundPosCells - Vec2(groundWidthCells * 0.5f, groundHeightCells * 0.5f);
		Vec2 maxs = groundPosCells + Vec2(groundWidthCells * 0.5f, groundHeightCells * 0.5f);
		AABB2 groundBox(mins, maxs);

		// 添加矩形顶点（绿色半透明）
		AddVertsForAABB2D(debugVerts, groundBox, Rgba8(0, 255, 0, 128));
	}

	// ========== 渲染方块 ==========
	for (b2BodyId boxBodyId : m_testBoxBodies) {
		if (B2_IS_NULL(boxBodyId)) continue;

		b2Vec2 boxPosMeters = b2Body_GetPosition(boxBodyId);
		b2Rot boxRot = b2Body_GetRotation(boxBodyId);
		Vec2 boxPosCells = PhysicsToCell(boxPosMeters);

		// 方块尺寸
		float boxSizeCells = 32.0f;
		Vec2 boxHalfDims(boxSizeCells * 0.5f, boxSizeCells * 0.5f);

		// 计算旋转角度（弧度转角度）
		float boxAngleDegrees = atan2f(boxRot.s, boxRot.c) * 180.0f / 3.14159f;

		// 创建iBasis（方向向量）
		Vec2 boxIBasis = Vec2::MakeFromPolarDegrees(boxAngleDegrees);

		// 创建OBB2
		OBB2 boxOBB(boxPosCells, boxIBasis, boxHalfDims);

		// 添加顶点（黄色半透明）
		AddVertsForOBB2D(debugVerts, boxOBB, Rgba8(255, 255, 0, 200));
	}

	// 渲染所有调试顶点
	if (!debugVerts.empty()) {
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray((int)debugVerts.size(), debugVerts.data());
	}
}

void SandboxMap::UpdateCellsPhysInChunk()
{
	//============NEW VERSION============
	for (auto& row : m_chunks) {
		for (auto& chunk : row) {
			chunk->ResetUpdateFlags();
		}
	}

	int phases[4] = { 0, 1, 2, 3 };

	switch (m_updateOrder) {
	case UpdateOrder::FIXED:
		// 保持 0, 1, 2, 3
		break;

	case UpdateOrder::ROTATING:
	{
		static int startPhase = 0;
		for (int i = 0; i < 4; ++i) {
			phases[i] = (startPhase + i) % 4;
		}
		startPhase = (startPhase + 1) % 4;
	}
	break;

	case UpdateOrder::RANDOM:
	{
		// 随机打乱
		for (int i = 3; i > 0; --i) {
			int j = rand() % (i + 1);
			std::swap(phases[i], phases[j]);
		}
	}
	break;
	}

	// 按选定顺序更新
	for (int i = 0; i < 4; ++i) {
		UpdateChunksOfPhase(phases[i]);
	}
}


bool SandboxMap::IsInBounds(int x, int y) const
{
	if (x >= 0 && x < m_mapSize.x && y >= 0 && y < m_mapSize.y)
	{
		return true;
	}
	return false;
}

const char* SandboxMap::GetMaterialTypeName(CellMatType type) const
{
	switch (type)
	{
	case CellMatType::MAT_EMPTY: return "Empty";
	case CellMatType::MAT_SAND:  return "Sand";
	case CellMatType::MAT_WATER: return "Water";
	case CellMatType::MAT_STONE: return "Stone";
	default: return "Unknown";
	}
}

void SandboxMap::UpdateCellsChemicalInChunk()
{
	for (auto& row : m_chunks) {
		for (auto& chunk : row) {
			UpdateSingleChunkChemical(chunk);
		}
	}
}

void SandboxMap::UpdateSingleChunkChemical(CellChunk* chunk)
{
	for (int localY = 0; localY < CHUNK_SIZE; ++localY)
	{
		for (int localX = 0; localX < CHUNK_SIZE; localX++) 
		{
			Cell& cell = chunk->GetLocalCell(localX, localY);
			if (cell.IsEmpty()) continue;

			IntVec2 worldPos = chunk->LocalToWorld(localX, localY);
			CellMatDef curCellDef = CellMatManager::GetMaterialDef(cell.m_type);

			// High Temperature
			if (curCellDef.m_isHighTemp) 
				chunk->MarkDirty();

			// Life Countdown
			if (cell.m_lifeCountDown == 0)
			{
				cell.SetToType(curCellDef.m_lifeEndMatType);
				curCellDef = CellMatManager::GetMaterialDef(cell.m_type);
			}
			else if (cell.m_lifeCountDown > 0)
			{
				cell.m_lifeCountDown--;
			}
			

			// Flammable
			IntVec2 directions[8] = { IntVec2(1,0),IntVec2(-1,0),IntVec2(0,1),IntVec2(0,-1),
			IntVec2(1,1),IntVec2(-1,1),IntVec2(1,1),IntVec2(-1,-1) };
			if (curCellDef.m_isFlammable)
			{
				if (cell.m_flameCountDown <= 0)
				{
					cell.SetToType(curCellDef.m_flammableType);
					chunk->MarkDirty();
					curCellDef = CellMatManager::GetMaterialDef(cell.m_type);
				}
				else
				{
					for (int i = 0; i < 8; i++)
					{
						IntVec2 neighborWorldPos = worldPos + directions[i];
						if (IsInBounds(neighborWorldPos.x, neighborWorldPos.y))
						{
							CellMatDef neighborCellDef = CellMatManager::GetMaterialDef(GetCell(neighborWorldPos.x, neighborWorldPos.y).m_type);
							if (neighborCellDef.m_isHighTemp)
							{
								cell.m_flameCountDown--;
							}
						}
					}
				}
			}
			
			// Dissolve
			if (curCellDef.m_isDissolve)
			{
				if (cell.m_dissolveCountDown <= 0)
				{
					cell.SetToType(curCellDef.m_dissolveType);
					chunk->MarkDirty();
					curCellDef = CellMatManager::GetMaterialDef(cell.m_type);
				}
				else
				{
					for (int i = 0; i < 8; i++)
					{
						IntVec2 neighborWorldPos = worldPos + directions[i];
						if (IsInBounds(neighborWorldPos.x, neighborWorldPos.y))
						{
							CellMatDef neighborCellDef = CellMatManager::GetMaterialDef(GetCell(neighborWorldPos.x, neighborWorldPos.y).m_type);
							if (neighborCellDef.m_physicsType == PhyType::PHY_LIQUID)
							{
								cell.m_dissolveCountDown--;
							}
						}
					}
				}
			}

			// Corrosion
			if (curCellDef.m_isCorroded)
			{
				if (cell.m_corrosionCountDown <= 0)
				{
					cell.SetToType(curCellDef.m_corrodeType);
					chunk->MarkDirty();
					curCellDef = CellMatManager::GetMaterialDef(cell.m_type);
				}
				else
				{
					for (int i = 0; i < 8; i++)
					{
						IntVec2 neighborWorldPos = worldPos + directions[i];
						if (IsInBounds(neighborWorldPos.x, neighborWorldPos.y))
						{
							CellMatDef neighborCellDef = CellMatManager::GetMaterialDef(GetCell(neighborWorldPos.x, neighborWorldPos.y).m_type);
							if (neighborCellDef.m_isAcid)
							{
								cell.m_corrosionCountDown--;
							}
						}
					}
				}
			}
		}
	}
}

void SandboxMap::CreateBox2dWorld()
{
	b2SetLengthUnitsPerMeter(CELLS_PER_METER);

	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = b2Vec2{ 0.0f, -GRAVITY_METERS };  

	worldDef.enableSleep = true;              // 允许物体休眠
	worldDef.enableContinuous = true;         // 启用连续碰撞检测
	worldDef.contactHertz = 30.0f;           // 接触刚度
	worldDef.contactDampingRatio = 10.0f;    // 接触阻尼

	m_b2WorldId = b2CreateWorld(&worldDef);

	if (B2_IS_NULL(m_b2WorldId))
	{
		ERROR_AND_DIE("Failed to create Box2D world!");
	}
}

CellChunk* SandboxMap::GetChunk(int chunkX, int chunkY)
{
	if (!IsChunkIndexValid(chunkX, chunkY)) {
		return nullptr;
	}
	return m_chunks[chunkY][chunkX];
}

CellChunk const* SandboxMap::GetChunk(int chunkX, int chunkY) const
{
	if (!IsChunkIndexValid(chunkX, chunkY)) {
		return nullptr;
	}
	return m_chunks[chunkY][chunkX];
}

CellChunk* SandboxMap::GetChunkByWorldPos(int worldX, int worldY)
{
	IntVec2 chunkIndex = CellChunk::WorldToChunkIndex(worldX, worldY);
	return GetChunk(chunkIndex.x, chunkIndex.y);
}

bool SandboxMap::IsChunkIndexValid(int chunkX, int chunkY) const
{
	return chunkX >= 0 && chunkX < m_chunkGridSize.x &&
		chunkY >= 0 && chunkY < m_chunkGridSize.y;
}

