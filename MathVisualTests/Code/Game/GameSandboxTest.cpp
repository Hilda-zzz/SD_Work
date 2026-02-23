#include "GameSandboxTest.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Window/Window.hpp"
#include "App.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include <queue>

// 常量定义
constexpr float CAMERA_MOVE_SPEED = 300.0f;
constexpr float GRID_LINE_THICKNESS = 1.0f;
constexpr int DEFAULT_GRID_WIDTH = 120;
constexpr int DEFAULT_GRID_HEIGHT = 67;
constexpr float DEFAULT_CELL_SIZE = 10.0f;

GameSandboxTest::GameSandboxTest()
{
	IntVec2 windowDimension = g_theWindow->GetClientDimensions();
	m_screenCamera.SetViewport(AABB2(Vec2(0.f, 0.f), Vec2((float)windowDimension.x, (float)windowDimension.y)));

	// Calculate map center
	Vec2 mapCenter = Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y * 0.5f);

	// Calculate view size (10% larger than map size)
	float viewWidth = SCREEN_SIZE_X * 1.1f;
	float viewHeight =SCREEN_SIZE_Y * 1.1f;

	// Calculate view bounds centered on map
	Vec2 viewMins = mapCenter - Vec2(viewWidth * 0.5f, viewHeight * 0.5f);
	Vec2 viewMaxs = mapCenter + Vec2(viewWidth * 0.5f, viewHeight * 0.5f);

	m_screenCamera.SetOrthographicView(viewMins, viewMaxs);
	//----------------------------------------------------------------------
	g_theInput->SetCursorMode(CursorMode::POINTER);
	g_theWindow->SetCursorVisible(true);

	m_gameClock = new Clock();

	// 初始化网格
	m_gridSize = IntVec2(DEFAULT_GRID_WIDTH, DEFAULT_GRID_HEIGHT);
	m_cellSize = DEFAULT_CELL_SIZE;
	m_gridBounds = AABB2(0.0f, 0.0f,
		(float)m_gridSize.x * m_cellSize,
		(float)m_gridSize.y * m_cellSize);

	// 创建网格
	m_grid = std::vector<std::vector<TestCell>>(m_gridSize.y,
		std::vector<TestCell>(m_gridSize.x));

	// 初始化颜色调色板
	m_colorPalette = {
		Rgba8::WHITE,
		Rgba8::RED,
		Rgba8::GREEN,
		Rgba8::BLUE,
		Rgba8::YELLOW,
		Rgba8::CYAN,
		Rgba8(128, 128, 128), // 灰色
		Rgba8(64, 64, 64),    // 深灰色
	};

	m_currentColor = m_colorPalette[0];
}

GameSandboxTest::~GameSandboxTest()
{
	g_systemClock->RemoveChild(m_gameClock);
	delete m_gameClock;
	m_gameClock = nullptr;
}

void GameSandboxTest::Update()
{
	float deltaTime = (float)m_gameClock->GetDeltaSeconds();

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->m_isQuitting = true;
		return;
	}

	// 功能快捷键
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		RandomizeGrid();
	}

	if (g_theInput->WasKeyJustPressed('C'))
	{
		ClearGrid();
	}

	// 切换绘制模式
	if (g_theInput->WasKeyJustPressed('1'))
		m_currentDrawMode = DrawMode::SINGLE_CLICK;
	if (g_theInput->WasKeyJustPressed('2'))
		m_currentDrawMode = DrawMode::BRUSH_SMALL;
	if (g_theInput->WasKeyJustPressed('3'))
		m_currentDrawMode = DrawMode::BRUSH_MEDIUM;
	if (g_theInput->WasKeyJustPressed('4'))
		m_currentDrawMode = DrawMode::BRUSH_LARGE;
	if (g_theInput->WasKeyJustPressed('5'))
		m_currentDrawMode = DrawMode::LINE_DRAW;
	if (g_theInput->WasKeyJustPressed('6'))
		m_currentDrawMode = DrawMode::FLOOD_FILL;

	// 颜色切换
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTARROW))
	{
		m_selectedColorIndex = (m_selectedColorIndex - 1 + (int)m_colorPalette.size()) % (int)m_colorPalette.size();
		m_currentColor = m_colorPalette[m_selectedColorIndex];
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTARROW))
	{
		m_selectedColorIndex = (m_selectedColorIndex + 1) % (int)m_colorPalette.size();
		m_currentColor = m_colorPalette[m_selectedColorIndex];
	}

	UpdateDeveloperCheats(deltaTime);
	UpdateInput(deltaTime);
	HandleMouseInput();
	UpdateCamera(deltaTime);

	// 统计活跃格子数量
	m_activeCells = 0;
	for (int y = 0; y < m_gridSize.y; y++)
	{
		for (int x = 0; x < m_gridSize.x; x++)
		{
			if (m_grid[y][x].IsActive())
				m_activeCells++;
		}
	}
}

void GameSandboxTest::Renderer() const
{
	g_theRenderer->BeginCamera(m_screenCamera);

	RenderGrid();
	RenderUI();

	g_theRenderer->EndCamera(m_screenCamera);
}

void GameSandboxTest::UpdateCamera(float deltaTime)
{
	IntVec2 dimensions = g_theWindow->GetClientDimensions();
	m_screenCamera.SetViewport(AABB2(Vec2::ZERO, Vec2((float)dimensions.x, (float)dimensions.y)));

	// 简单的相机控制
	Vec2 cameraPos = m_screenCamera.GetOrthoBottomLeft();
	Vec2 cameraSize = m_screenCamera.GetOrthoTopRight() - cameraPos;

	// WASD 移动相机
	if (g_theInput->IsKeyDown('W'))
		cameraPos.y += CAMERA_MOVE_SPEED * deltaTime;
	if (g_theInput->IsKeyDown('S'))
		cameraPos.y -= CAMERA_MOVE_SPEED * deltaTime;
	if (g_theInput->IsKeyDown('A'))
		cameraPos.x -= CAMERA_MOVE_SPEED * deltaTime;
	if (g_theInput->IsKeyDown('D'))
		cameraPos.x += CAMERA_MOVE_SPEED * deltaTime;

	m_screenCamera.SetOrthographicView(cameraPos, cameraPos + cameraSize);
}

void GameSandboxTest::UpdateInput(float deltaTime)
{
	UNUSED(deltaTime);

	// 更新鼠标在网格中的位置
	Vec2 mousePos = g_theWindow->GetNormalizedMouseUV();
	AABB2 worldUV = AABB2(m_screenCamera.GetOrthoBottomLeft().x, m_screenCamera.GetOrthoBottomLeft().y,
		m_screenCamera.GetOrthoTopRight().x, m_screenCamera.GetOrthoTopRight().y);
	Vec2 worldMousePos = worldUV.GetPointAtUV(mousePos);

	IntVec2 gridPos = ScreenToGrid(worldMousePos);
	m_mouseGridPos = Vec2((float)gridPos.x, (float)gridPos.y);
}

void GameSandboxTest::RenderGrid() const
{
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);

	m_gridVerts.clear();

	// 绘制活跃的格子
	for (int y = 0; y < m_gridSize.y; y++)
	{
		for (int x = 0; x < m_gridSize.x; x++)
		{
			if (m_grid[y][x].IsActive())
			{
				Vec2 cellBottomLeft = Vec2((float)x * m_cellSize, (float)y * m_cellSize);
				Vec2 cellTopRight = cellBottomLeft + Vec2(m_cellSize, m_cellSize);
				AABB2 cellBounds(cellBottomLeft, cellTopRight);

				AddVertsForAABB2D(m_gridVerts, cellBounds, m_grid[y][x].m_color);
			}
		}
	}

	// 绘制网格线（可选，可以通过按键切换）
// 	if (g_theInput->IsKeyDown('G'))
// 	{
		// 垂直线
		for (int x = 0; x <= m_gridSize.x; x++)
		{
			float xPos = (float)x * m_cellSize;
			Vec2 start(xPos, 0.0f);
			Vec2 end(xPos, (float)m_gridSize.y * m_cellSize);
			AddVertsForLineSegment2D(m_gridVerts, start, end, GRID_LINE_THICKNESS, Rgba8(100, 100, 100, 100));
		}

		// 水平线
		for (int y = 0; y <= m_gridSize.y; y++)
		{
			float yPos = (float)y * m_cellSize;
			Vec2 start(0.0f, yPos);
			Vec2 end((float)m_gridSize.x * m_cellSize, yPos);
			AddVertsForLineSegment2D(m_gridVerts, start, end, GRID_LINE_THICKNESS, Rgba8(100, 100, 100, 100));
		}
	//}

	// 高亮当前鼠标所在格子
	IntVec2 mouseGrid = IntVec2((int)m_mouseGridPos.x, (int)m_mouseGridPos.y);
	if (IsValidGridPosition(mouseGrid))
	{
		Vec2 cellBottomLeft = Vec2((float)mouseGrid.x * m_cellSize, (float)mouseGrid.y * m_cellSize);
		Vec2 cellTopRight = cellBottomLeft + Vec2(m_cellSize, m_cellSize);
		AABB2 cellBounds(cellBottomLeft, cellTopRight);

		AddVertsForAABBWire2D(m_gridVerts, cellBounds, Rgba8::WHITE, 2.0f,true);
	}

	g_theRenderer->DrawVertexArray(m_gridVerts);
}

void GameSandboxTest::RenderUI() const
{
	g_theRenderer->SetModelConstants();
	m_uiVerts.clear();

	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	// 标题和说明
	font->AddVertsForTextInBox2D(m_uiVerts, "Sandbox Grid Test Environment",
		AABB2(Vec2(10.f, SCREEN_SIZE_Y - 30.f), Vec2(800.f, SCREEN_SIZE_Y - 10.f)),
		20.f, Rgba8::YELLOW, 0.7f, Vec2(0.f, 0.f));

	// 控制说明
	std::string controls = "Controls: 1-6 (Draw Modes), C (Clear), F8 (Random), G (Grid Lines), WASD (Camera), Arrows (Color)";
	font->AddVertsForTextInBox2D(m_uiVerts, controls,
		AABB2(Vec2(10.f, SCREEN_SIZE_Y - 55.f), Vec2(1200.f, SCREEN_SIZE_Y - 35.f)),
		12.f, Rgba8::CYAN, 0.7f, Vec2(0.f, 0.f));

	// 当前模式显示
	std::string modeText = "Mode: ";
	switch (m_currentDrawMode)
	{
	case DrawMode::SINGLE_CLICK: modeText += "Single Click (1)"; break;
	case DrawMode::BRUSH_SMALL: modeText += "Small Brush (2)"; break;
	case DrawMode::BRUSH_MEDIUM: modeText += "Medium Brush (3)"; break;
	case DrawMode::BRUSH_LARGE: modeText += "Large Brush (4)"; break;
	case DrawMode::LINE_DRAW: modeText += "Line Draw (5)"; break;
	case DrawMode::FLOOD_FILL: modeText += "Flood Fill (6)"; break;
	}

	font->AddVertsForTextInBox2D(m_uiVerts, modeText,
		AABB2(Vec2(10.f, SCREEN_SIZE_Y - 80.f), Vec2(400.f, SCREEN_SIZE_Y - 60.f)),
		15.f, Rgba8::WHITE, 0.7f, Vec2(0.f, 0.f));

	// 统计信息
	std::string stats = Stringf("Grid: %dx%d | Active Cells: %d | Mouse: (%.0f, %.0f)",
		m_gridSize.x, m_gridSize.y, m_activeCells, m_mouseGridPos.x, m_mouseGridPos.y);
	font->AddVertsForTextInBox2D(m_uiVerts, stats,
		AABB2(Vec2(10.f, SCREEN_SIZE_Y - 105.f), Vec2(600.f, SCREEN_SIZE_Y - 85.f)),
		12.f, Rgba8::GREEN, 0.7f, Vec2(0.f, 0.f));

	// 颜色调色板显示
// 	float paletteStartX = 10.f;
// 	float paletteY = SCREEN_SIZE_Y - 140.f;
// 	float colorBoxSize = 60.f;
// 
// 	for (int i = 0; i < (int)m_colorPalette.size(); i++)
// 	{
// 		float x = paletteStartX + i * (colorBoxSize + 5.f);
// 		AABB2 colorBox(Vec2(x, paletteY), Vec2(x + colorBoxSize, paletteY + colorBoxSize));
// 
// 		AddVertsForAABB2D(m_uiVerts, colorBox, m_colorPalette[i]);
// 
// 		// 高亮当前选中的颜色
// 		if (i == m_selectedColorIndex)
// 		{
// 			AddVertsForAABBWire2D(m_uiVerts, colorBox, Rgba8::WHITE, 3.0f,true);
// 		}
// 		else
// 		{
// 			AddVertsForAABBWire2D(m_uiVerts, colorBox, Rgba8::HILDA, 1.0f,true);
// 		}
// 	}

	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(m_uiVerts);
}

void GameSandboxTest::HandleMouseInput()
{
	Vec2 mousePos = g_theWindow->GetNormalizedMouseUV();
	AABB2 worldUV = AABB2(m_screenCamera.GetOrthoBottomLeft().x, m_screenCamera.GetOrthoBottomLeft().y,
		m_screenCamera.GetOrthoTopRight().x, m_screenCamera.GetOrthoTopRight().y);
	Vec2 worldMousePos = worldUV.GetPointAtUV(mousePos);
	IntVec2 gridPos = ScreenToGrid(worldMousePos);

	if (!IsValidGridPosition(gridPos))
		return;

	bool leftMouseDown = g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE);
	bool leftMouseJustPressed = g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE);
	bool rightMouseJustPressed = g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE);

	// 右键清除单个格子
	if (rightMouseJustPressed)
	{
		m_grid[gridPos.y][gridPos.x].SetInactive();
		return;
	}

	// 左键绘制
	if (leftMouseJustPressed || (leftMouseDown && gridPos != m_lastDrawPos))
	{
		switch (m_currentDrawMode)
		{
		case DrawMode::SINGLE_CLICK:
			SetPixel(gridPos, m_currentColor);
			break;

		case DrawMode::BRUSH_SMALL:
			DrawBrush(gridPos, 1, m_currentColor);
			break;

		case DrawMode::BRUSH_MEDIUM:
			DrawBrush(gridPos, 2, m_currentColor);
			break;

		case DrawMode::BRUSH_LARGE:
			DrawBrush(gridPos, 3, m_currentColor);
			break;

		case DrawMode::LINE_DRAW:
			if (leftMouseJustPressed)
			{
				if (!m_isDrawing)
				{
					m_isDrawing = true;
					m_lastDrawPos = gridPos;
				}
				else
				{
					DrawLine(m_lastDrawPos, gridPos, m_currentColor);
					m_isDrawing = false;
				}
			}
			break;

		case DrawMode::FLOOD_FILL:
			if (leftMouseJustPressed)
			{
				FloodFill(gridPos, m_currentColor);
			}
			break;
		}

		if (m_currentDrawMode != DrawMode::LINE_DRAW)
		{
			m_lastDrawPos = gridPos;
		}
	}
}

IntVec2 GameSandboxTest::ScreenToGrid(Vec2 screenPos) const
{
	int x = (int)(screenPos.x / m_cellSize);
	int y = (int)(screenPos.y / m_cellSize);
	return IntVec2(x, y);
}

Vec2 GameSandboxTest::GridToScreen(IntVec2 gridPos) const
{
	return Vec2((float)gridPos.x * m_cellSize, (float)gridPos.y * m_cellSize);
}

bool GameSandboxTest::IsValidGridPosition(IntVec2 gridPos) const
{
	return gridPos.x >= 0 && gridPos.x < m_gridSize.x &&
		gridPos.y >= 0 && gridPos.y < m_gridSize.y;
}

void GameSandboxTest::SetPixel(IntVec2 gridPos, Rgba8 color)
{
	if (IsValidGridPosition(gridPos))
	{
		m_grid[gridPos.y][gridPos.x].SetActive(color);
	}
}

void GameSandboxTest::DrawBrush(IntVec2 centerPos, int brushSize, Rgba8 color)
{
	for (int dy = -brushSize; dy <= brushSize; dy++)
	{
		for (int dx = -brushSize; dx <= brushSize; dx++)
		{
			IntVec2 pos = centerPos + IntVec2(dx, dy);
			if (dx * dx + dy * dy <= brushSize * brushSize)
			{
				SetPixel(pos, color);
			}
		}
	}
}

void GameSandboxTest::DrawLine(IntVec2 start, IntVec2 end, Rgba8 color)
{
	// 计算总距离用于插值
	int dx = abs(end.x - start.x);
	int dy = abs(end.y - start.y);
	float totalDistance = sqrtf((float)(dx * dx + dy * dy));

	// 预定义起点和终点颜色
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::GREEN;

	// 首先绘制起点
	SetPixel(start, startColor);

	// 如果起点和终点相同，只绘制一个点
	if (start.x == end.x && start.y == end.y) {
		return;
	}

	// 使用BresenhamLineExcludeStart处理除起点外的所有点
	float currentDistance = 0.0f;
	IntVec2 previousPos = start;

	BresenhamLineExcludeStart(start.x, start.y, end.x, end.y, [&](int x, int y) {
		IntVec2 currentPos(x, y);

		// 计算从起点到当前点的累积距离
		float stepDist = sqrtf((float)((currentPos.x - previousPos.x) * (currentPos.x - previousPos.x) +
			(currentPos.y - previousPos.y) * (currentPos.y - previousPos.y)));
		currentDistance += stepDist;

		// 计算插值参数 t (0.0 到 1.0)
		float t = (totalDistance > 0.0f) ? (currentDistance / totalDistance) : 0.0f;
		t = GetClamped(t, 0.0f, 1.0f);

		// 线性插值颜色
		Rgba8 interpolatedColor;
		interpolatedColor.r = (unsigned char)((1.0f - t) * startColor.r + t * endColor.r);
		interpolatedColor.g = (unsigned char)((1.0f - t) * startColor.g + t * endColor.g);
		interpolatedColor.b = (unsigned char)((1.0f - t) * startColor.b + t * endColor.b);
		interpolatedColor.a = (unsigned char)((1.0f - t) * startColor.a + t * endColor.a);

		SetPixel(currentPos, interpolatedColor);

		// 更新上一个位置
		previousPos = currentPos;
		});
}

void GameSandboxTest::FloodFill(IntVec2 startPos, Rgba8 newColor)
{
	if (!IsValidGridPosition(startPos))
		return;

	Rgba8 originalColor = m_grid[startPos.y][startPos.x].m_color;
	bool originalActive = m_grid[startPos.y][startPos.x].IsActive();

	// 如果颜色相同，不需要填充
	if (originalActive && originalColor.r == newColor.r &&
		originalColor.g == newColor.g && originalColor.b == newColor.b)
		return;

	std::queue<IntVec2> fillQueue;
	fillQueue.push(startPos);

	while (!fillQueue.empty())
	{
		IntVec2 pos = fillQueue.front();
		fillQueue.pop();

		if (!IsValidGridPosition(pos))
			continue;

		TestCell& cell = m_grid[pos.y][pos.x];

		// 检查是否应该填充这个格子
		bool shouldFill = false;
		if (!originalActive && !cell.IsActive())
			shouldFill = true;
		else if (originalActive && cell.IsActive() &&
			cell.m_color.r == originalColor.r &&
			cell.m_color.g == originalColor.g &&
			cell.m_color.b == originalColor.b)
			shouldFill = true;

		if (shouldFill)
		{
			cell.SetActive(newColor);

			// 添加相邻格子到队列
			fillQueue.push(IntVec2(pos.x + 1, pos.y));
			fillQueue.push(IntVec2(pos.x - 1, pos.y));
			fillQueue.push(IntVec2(pos.x, pos.y + 1));
			fillQueue.push(IntVec2(pos.x, pos.y - 1));
		}
	}
}

void GameSandboxTest::ClearGrid()
{
	for (int y = 0; y < m_gridSize.y; y++)
	{
		for (int x = 0; x < m_gridSize.x; x++)
		{
			m_grid[y][x].SetInactive();
		}
	}
}

void GameSandboxTest::RandomizeGrid()
{
	for (int y = 0; y < m_gridSize.y; y++)
	{
		for (int x = 0; x < m_gridSize.x; x++)
		{
			if (rand() % 10 < 3) // 30% 概率设置格子
			{
				int colorIndex = rand() % (int)m_colorPalette.size();
				m_grid[y][x].SetActive(m_colorPalette[colorIndex]);
			}
			else
			{
				m_grid[y][x].SetInactive();
			}
		}
	}
}