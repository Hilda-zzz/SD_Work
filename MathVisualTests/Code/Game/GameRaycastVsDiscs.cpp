#include "GameRaycastVsDiscs.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Input/InputSystem.hpp"
//#include "Engine/Renderer/Renderer.hpp"
#include "App.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "GameCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Clock.hpp"

//extern Clock* g_systemClock;
constexpr float SPEED = 180.f;
GameRaycastVsDiscs::GameRaycastVsDiscs()
{
	m_gameClock = new Clock();
	m_arrow = RaycastArrow(Vec2(100.f, 50.f), Vec2(2.f, 1.f), 1400.f);
	//m_final_result = RaycastResult2D(Vec2(100.f, 50.f), Vec2(2.f, 1.f), 1400.f);
	for (int i = 0; i < 10; i++)
	{
		float center_x = m_rng.RollRandomFloatInRange(100.f, 1500.f);
		float ceneter_y = m_rng.RollRandomFloatInRange(100.f, 700.f);
		float radius = m_rng.RollRandomFloatInRange(20.f, 150.f);
		DiscObstacle* disc = new DiscObstacle(Vec2(center_x, ceneter_y), radius);
		m_discs.push_back(disc);
	}
}

GameRaycastVsDiscs::~GameRaycastVsDiscs()
{
	for (int i = 0; i < 10; i++)
	{
		delete m_discs[i];
		m_discs[i] = nullptr;
	}

	g_systemClock->RemoveChild(m_gameClock);
	delete m_gameClock;
	m_gameClock = nullptr;
}

void GameRaycastVsDiscs::Update()
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

	UpdateKBInputForRaycastMode(deltaTime,m_arrow,SPEED);

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		RerandomDiscs();
	}
	m_index = -1;
	m_final_result.m_didImpact = false;
	m_final_result.m_impactDist = 100000;
	Vec2 startPoint = m_arrow.m_startPos;
	Vec2 fwdNormal = (m_arrow.m_endPos_whole - m_arrow.m_startPos).GetNormalized();
	float maxDist = (m_arrow.m_endPos_whole - m_arrow.m_startPos).GetLength();

	for (int i = 0; i < 10; i++)
	{
		RaycastResult2D result = RaycastVsDisc2D(startPoint, fwdNormal, maxDist,m_discs[i]->m_center, m_discs[i]->m_radius);
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

void GameRaycastVsDiscs::Renderer() const
{
	g_theRenderer->BeginCamera(m_screenCamera);

	for (int i = 0; i < 10; i++)
	{
		m_discs[i]->m_isRaycast = false;
		if (m_index == i)
		{
			m_discs[i]->m_isRaycast = true;
		}
		m_discs[i]->Render();
	}

	if (m_final_result.m_didImpact)
	{
		std::vector<Vertex_PCU> arrow_verts;
		AddVertsForArrow2D(arrow_verts, m_arrow.m_startPos, m_arrow.m_endPos_whole, 15.f, 2.f, Rgba8(130, 130, 130));
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray(arrow_verts);

		std::vector<Vertex_PCU> arrow_verts2;
		AddVertsForArrow2D(arrow_verts2, m_final_result.m_impactPos, m_final_result.m_impactPos+ m_final_result.m_impactNormal * 50.f, 15.f, 2.f, Rgba8::CYAN);
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray(arrow_verts2);

		std::vector<Vertex_PCU> arrow_verts3;
		AddVertsForArrow2D(arrow_verts3, m_arrow.m_startPos,m_final_result.m_impactPos, 15.f, 2.f, Rgba8(192,79,30));
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
	font->AddVertsForTextInBox2D(title, "Mode (F6/F7 for prev/next): Raycast vs. Disc2D", AABB2(Vec2(10.f, 770.f), Vec2(1600.f, 790.f)), 15.f, Rgba8(200, 200, 0), 0.7f, Vec2(0.f, 0.f));
	font->AddVertsForTextInBox2D(title, "F8 to randomize; LMB/RMB set ray start/end; ESDF move start, IJKL move end, arrows move ray,hold T=slow", AABB2(Vec2(10.f, 745.f), Vec2(1600.f, 765.f)), 15.f, Rgba8(0, 200, 200), 0.7f, Vec2(0.f, 0.f));
	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(title);

	g_theRenderer->EndCamera(m_screenCamera);
}
void GameRaycastVsDiscs::RerandomDiscs()
{
	for (int i = 0; i < 10; i++)
	{
		float center_x = m_rng.RollRandomFloatInRange(100.f, 1500.f);
		float ceneter_y = m_rng.RollRandomFloatInRange(100.f, 700.f);
		float radius = m_rng.RollRandomFloatInRange(20.f, 150.f);
		m_discs[i]->m_center =Vec2(center_x, ceneter_y);
		m_discs[i]->m_radius = radius;
	}
}
void GameRaycastVsDiscs::UpdateCamera(float deltaTime)
{
	UNUSED(deltaTime);
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	m_worldUV = AABB2(m_screenCamera.GetOrthoBottomLeft().x, m_screenCamera.GetOrthoBottomLeft().y,
		m_screenCamera.GetOrthoTopRight().x, m_screenCamera.GetOrthoTopRight().y);
}

