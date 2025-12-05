// GameMapDebugRenderer.hpp
// Debug visualization for GameMap super chunks and boundaries

#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>

class GameMap;
class Camera;
struct Vertex_PCU;

class GameMapDebugRenderer {
public:
	GameMapDebugRenderer(GameMap* map);
	~GameMapDebugRenderer();

	void Render(Camera const& camera) const;
	void RenderDebugUI();  // ImGui debug control panel

	// Toggle options
	void SetDrawSuperChunkBounds(bool draw) { m_drawSuperChunkBounds = draw; }
	void SetDrawChunkBounds(bool draw) { m_drawChunkBounds = draw; }
	void SetDrawActiveOnly(bool activeOnly) { m_drawActiveOnly = activeOnly; }
	void SetHighlightPlayerLocation(bool highlight) { m_highlightPlayerLocation = highlight; }

	bool GetDrawSuperChunkBounds() const { return m_drawSuperChunkBounds; }
	bool GetDrawChunkBounds() const { return m_drawChunkBounds; }
	bool GetDrawActiveOnly() const { return m_drawActiveOnly; }
	bool GetHighlightPlayerLocation() const { return m_highlightPlayerLocation; }

	// Additional getters/setters for new debug options
	void SetDrawFrustumCulling(bool draw) { m_drawFrustumCulling = draw; }
	bool GetDrawFrustumCulling() const { return m_drawFrustumCulling; }

	void SetDrawGeneratedPixels(bool draw) { m_drawGeneratedPixels = draw; }
	bool GetDrawGeneratedPixels() const { return m_drawGeneratedPixels; }

	void SetDrawPixelGrid(bool draw) { m_drawPixelGrid = draw; }
	bool GetDrawPixelGrid() const { return m_drawPixelGrid; }

	void SetPixelDrawMode(int mode) { m_pixelDrawMode = mode; }
	int GetPixelDrawMode() const { return m_pixelDrawMode; }

private:
	void RenderSuperChunkBounds(Camera const& camera) const;
	void RenderChunkBounds(Camera const& camera) const;
	void RenderPlayerLocationHighlight(Camera const& camera) const;

	// ⭐ 新增：渲染函数
	void RenderGeneratedPixels(Camera const& camera) const;
	void RenderPixelGrid(Camera const& camera) const;

	void RenderFrustumCulling(Camera const& camera) const;

private:
	GameMap* m_map;
	bool m_drawSuperChunkBounds;
	bool m_drawChunkBounds;
	bool m_drawActiveOnly;
	bool m_highlightPlayerLocation;
	bool m_drawFrustumCulling = true;

	// ⭐ 新增：绘制选项
	bool m_drawGeneratedPixels = true;        // 是否绘制生成的pixels
	bool m_drawPixelGrid = true;              // 是否绘制pixel网格线
	int m_pixelDrawMode = 0;
};