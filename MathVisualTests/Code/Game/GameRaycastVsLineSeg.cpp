#include "GameRaycastVsLineSeg.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/GameCommon.hpp"
#include "App.hpp"
#include "Engine/Window/Window.hpp"
#include "Game/LineSegObstacle.hpp"
#include <Engine/Core/VertexUtils.hpp>

//extern Clock* g_systemClock;
constexpr float SPEED = 180.f;
GameRaycastVsLineSeg::GameRaycastVsLineSeg()
{
	g_theInput->SetCursorMode(CursorMode::POINTER);
	g_theWindow->SetCursorVisible(true);

	m_gameClock = new Clock();
	m_arrow = RaycastArrow(Vec2(100.f, 50.f), Vec2(2.f, 1.f), 1400.f);
	//m_final_result = RaycastResult2D(Vec2(100.f, 50.f), Vec2(2.f, 1.f), 1400.f);
	for (int i = 0; i < 10; i++)
	{
		float start_x = m_rng.RollRandomFloatInRange(100.f, 1500.f);
		float start_y = m_rng.RollRandomFloatInRange(100.f, 750.f);
		float dir_x= m_rng.RollRandomFloatInRange(-1.f, 1.f);
		float dir_y = m_rng.RollRandomFloatInRange(-1.f, 1.f);
		Vec2  dir = Vec2(dir_x, dir_y).GetNormalized();
		float len= m_rng.RollRandomFloatInRange(30.f, 200.f);
		Vec2 end = Vec2(start_x, start_y) + len * dir;
		LineSegment2 lineSeg =LineSegment2(Vec2(start_x,start_y), end);
		LineSegObstacle* lineSegObstacle= new LineSegObstacle(lineSeg);
		m_lineSegs.push_back(lineSegObstacle);
	}
}

GameRaycastVsLineSeg::~GameRaycastVsLineSeg()
{
	for (int i = 0; i < 10; i++)
	{
		delete m_lineSegs[i];
		m_lineSegs[i] = nullptr;
	}

	g_systemClock->RemoveChild(m_gameClock);
	delete m_gameClock;
	m_gameClock = nullptr;
}

void GameRaycastVsLineSeg::Update()
{
	float deltaTime = (float)m_gameClock->GetDeltaSeconds();
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->m_isQuitting = true;
		return;
	}

	UpdateDeveloperCheats(deltaTime);
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		Vec2 mousePos = g_theWindow->GetNormalizedMouseUV();
		m_arrow.m_startPos = m_worldUV.GetPointAtUV(mousePos);
	}
	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_MOUSE))
	{
		Vec2 mousePos = g_theWindow->GetNormalizedMouseUV();
		m_arrow.m_endPos_whole = m_worldUV.GetPointAtUV(mousePos);
	}

	UpdateKBInputForRaycastMode(deltaTime, m_arrow, SPEED);

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		RerandomLineSegs();
	}
	m_index = -1;
	m_final_result.m_didImpact = false;
	m_final_result.m_impactDist = 100000;
	Vec2 startPoint = m_arrow.m_startPos;
	Vec2 fwdNormal = (m_arrow.m_endPos_whole - m_arrow.m_startPos).GetNormalized();
	float maxDist = (m_arrow.m_endPos_whole - m_arrow.m_startPos).GetLength();

	for (int i = 0; i < 10; i++)
	{
		RaycastResult2D result = RaycastVsLineSegment2D(startPoint, fwdNormal, maxDist, m_lineSegs[i]->m_lineSeg.m_start, m_lineSegs[i]->m_lineSeg.m_end);
		if (result.m_didImpact)
		{
			if (result.m_impactDist < m_final_result.m_impactDist)
			{
				m_final_result = result;
				m_index = i;
			}
		}
	}

	UpdateCamera(deltaTime);
}

void GameRaycastVsLineSeg::Renderer() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->BindTexture(nullptr);
	for (int i = 0; i < 10; i++)
	{
		m_lineSegs[i]->m_isRaycast = false;
		if (m_index == i)
		{
			m_lineSegs[i]->m_isRaycast = true;
		}
		m_lineSegs[i]->Render();
	}

	if (m_final_result.m_didImpact)
	{
		std::vector<Vertex_PCU> arrow_verts;
		AddVertsForArrow2D(arrow_verts, m_arrow.m_startPos, m_arrow.m_endPos_whole, 15.f, 2.f, Rgba8(130, 130, 130));
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray(arrow_verts);

		std::vector<Vertex_PCU> arrow_verts2;
		AddVertsForArrow2D(arrow_verts2, m_final_result.m_impactPos, m_final_result.m_impactPos + m_final_result.m_impactNormal * 50.f, 15.f, 2.f, Rgba8::CYAN);
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray(arrow_verts2);

		std::vector<Vertex_PCU> arrow_verts3;
		AddVertsForArrow2D(arrow_verts3, m_arrow.m_startPos, m_final_result.m_impactPos, 15.f, 2.f, Rgba8(192, 79, 30));
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray(arrow_verts3);

		g_theRenderer->SetModelConstants();
		DebugDrawCircle(5.f, m_final_result.m_impactPos, Rgba8::WHITE);
	}
	else
	{
		std::vector<Vertex_PCU> arrow_verts;
		AddVertsForArrow2D(arrow_verts, m_arrow.m_startPos, m_arrow.m_endPos_whole, 15.f, 2.f, Rgba8::WHITE);
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray(arrow_verts);
	}

	std::vector<Vertex_PCU> title;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	font->AddVertsForTextInBox2D(title, "Mode (F6/F7 for prev/next): Raycast vs. LineSegment", AABB2(Vec2(10.f, 770.f), Vec2(1600.f, 790.f)), 15.f, Rgba8(200, 200, 0), 0.7f, Vec2(0.f, 0.f));
	font->AddVertsForTextInBox2D(title, "F8 to randomize; LMB/RMB set ray start/end; ESDF move start, IJKL move end, arrows move ray,hold T=slow", AABB2(Vec2(10.f, 745.f), Vec2(1600.f, 765.f)), 15.f, Rgba8(0, 200, 200), 0.7f, Vec2(0.f, 0.f));
	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(title);

	g_theRenderer->EndCamera(m_screenCamera);
}

void GameRaycastVsLineSeg::RerandomLineSegs()
{
	for (int i = 0; i < 10; i++)
	{
		delete m_lineSegs[i];
		float start_x = m_rng.RollRandomFloatInRange(100.f, 1500.f);
		float start_y = m_rng.RollRandomFloatInRange(100.f, 750.f);
		float dir_x = m_rng.RollRandomFloatInRange(-1.f, 1.f);
		float dir_y = m_rng.RollRandomFloatInRange(-1.f, 1.f);
		Vec2  dir = Vec2(dir_x, dir_y).GetNormalized();
		float len = m_rng.RollRandomFloatInRange(30.f, 200.f);
		Vec2 end = Vec2(start_x, start_y) + len * dir;
		LineSegment2 lineSeg = LineSegment2(Vec2(start_x, start_y), end);
		LineSegObstacle* lineSegObstacle = new LineSegObstacle(lineSeg);
		m_lineSegs[i]= lineSegObstacle;
	}
}

void GameRaycastVsLineSeg::UpdateCamera(float deltaTime)
{
	UNUSED(deltaTime);
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	m_worldUV = AABB2(m_screenCamera.GetOrthoBottomLeft().x, m_screenCamera.GetOrthoBottomLeft().y,
		m_screenCamera.GetOrthoTopRight().x, m_screenCamera.GetOrthoTopRight().y);
}

