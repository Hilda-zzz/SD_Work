#pragma once
#include "Game/Game.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>

class Clock;

// 简化的测试用格子结构
struct TestCell
{
	Rgba8 m_color = Rgba8::BLACK;
	bool m_isActive = false;

	void SetActive(Rgba8 color) { m_isActive = true; m_color = color; }
	void SetInactive() { m_isActive = false; m_color = Rgba8::BLACK; }
	bool IsActive() const { return m_isActive; }
};

class GameSandboxTest : public Game
{
public:
	GameSandboxTest();
	~GameSandboxTest();

	void Update() override;
	void Renderer() const override;
	void UpdateCamera(float deltaTime) override;

private:
	void UpdateInput(float deltaTime);
	void RenderGrid() const;
	void RenderUI() const;
	void HandleMouseInput();
	void ClearGrid();
	void RandomizeGrid();

	// 网格坐标转换
	IntVec2 ScreenToGrid(Vec2 screenPos) const;
	Vec2 GridToScreen(IntVec2 gridPos) const;
	bool IsValidGridPosition(IntVec2 gridPos) const;

	// 绘制模式
	enum class DrawMode
	{
		SINGLE_CLICK,   // 单点绘制
		BRUSH_SMALL,    // 小笔刷
		BRUSH_MEDIUM,   // 中笔刷
		BRUSH_LARGE,    // 大笔刷
		LINE_DRAW,      // 直线绘制
		FLOOD_FILL      // 填充
	};

	void SetPixel(IntVec2 gridPos, Rgba8 color);
	void DrawBrush(IntVec2 centerPos, int brushSize, Rgba8 color);
	void DrawLine(IntVec2 start, IntVec2 end, Rgba8 color);
	void FloodFill(IntVec2 startPos, Rgba8 newColor);

private:
	// 游戏时钟
	Clock* m_gameClock = nullptr;

	// 网格系统
	IntVec2 m_gridSize;
	float m_cellSize;
	std::vector<std::vector<TestCell>> m_grid;
	AABB2 m_gridBounds;

	// 相机
	Camera m_screenCamera;

	// 绘制相关
	DrawMode m_currentDrawMode = DrawMode::SINGLE_CLICK;
	Rgba8 m_currentColor = Rgba8::WHITE;
	bool m_isDrawing = false;
	IntVec2 m_lastDrawPos = IntVec2(-1, -1);

	// 颜色选择
	std::vector<Rgba8> m_colorPalette;
	int m_selectedColorIndex = 0;

	// UI相关
	mutable std::vector<Vertex_PCU> m_uiVerts;
	mutable std::vector<Vertex_PCU> m_gridVerts;

	// 统计信息
	int m_activeCells = 0;
	Vec2 m_mouseGridPos = Vec2::ZERO;
};


template<typename PointCallback>
inline void BresenhamLineExcludeStart(int x0, int y0, int x1, int y1, PointCallback&& callback)
{
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);

	// 如果起点和终点相同，没有其他点要处理
	if (dx == 0 && dy == 0) {
		return;
	}

	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;

	int x = x0, y = y0;

	// 跳过起点，直接执行第一步
	int e2 = err << 1;
	if (e2 > -dy) {
		err -= dy;
		x += sx;
	}
	if (e2 < dx) {
		err += dx;
		y += sy;
	}

	// 现在开始处理剩余点（包括终点）
	while (true) {
		callback(x, y);

		if (x == x1 && y == y1) break;

		e2 = err << 1;
		if (e2 > -dy) {
			err -= dy;
			x += sx;
		}
		if (e2 < dx) {
			err += dx;
			y += sy;
		}
	}
}
