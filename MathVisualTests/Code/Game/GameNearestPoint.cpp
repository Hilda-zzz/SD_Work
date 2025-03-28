#include "GameNearestPoint.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Window/Window.hpp"
#include "App.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

constexpr float AIMP_SPEED = 180.f;
Rgba8 NORMAL_COLOR = Rgba8(0, 100, 225, 255);
Rgba8 BRIGHT_COLOR = Rgba8(150, 100, 225, 255);

GameNearestPoint::GameNearestPoint()
{
	g_theInput->SetCursorMode(CursorMode::POINTER);
	g_theWindow->SetCursorVisible(true);

	m_gameClock = new Clock();
	ReRandomObject();
	//aim point position
	m_aimP_pos = Vec2(800.f, 400.f);
}

GameNearestPoint::~GameNearestPoint()
{
	g_systemClock->RemoveChild(m_gameClock);
	delete m_gameClock;
	m_gameClock = nullptr;
}

void GameNearestPoint::Update()
{
	float deltaTime = (float)m_gameClock->GetDeltaSeconds();
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->m_isQuitting = true;
		return;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		ReRandomObject();
	}
	UpdateDeveloperCheats(deltaTime);

	UpdateInput(deltaTime);

	SetColor(IsPointInsideDisc2D(m_aimP_pos,m_disc_pos,m_disc_radius), m_disc_color);
	SetColor(IsPointInsideAABB2D(m_aimP_pos,AABB2(m_aabb_mins,m_aabb_maxs) ), m_aabb_color);
	SetColor(IsPointInsideCapsule2D(m_aimP_pos,m_cap_start,m_cap_end,m_cap_radius ), m_cap_color);
	SetColor(IsPointInsideOBB2D(m_aimP_pos, OBB2(m_obb_center,m_obb_iBasisNormal,m_obb_halfDimensions)), m_obb_color);
	SetColor(IsPointInsideTriangle2D(m_aimP_pos, m_triangle), m_triangle_color);
	
	UpdateCamera(deltaTime);
}

void GameNearestPoint::Renderer() const
{
	g_theRenderer->BeginCamera(m_screenCamera);

	RenderBasicShape();
	
	g_theRenderer->SetModelConstants();
	Vec2 np_disc = GetNearestPointOnDisc2D(m_aimP_pos, m_disc_pos, m_disc_radius);
	DebugDrawLine(np_disc, m_aimP_pos, 2, Rgba8(255, 255, 255, 30));
	DebugDrawCircle(5.f, np_disc, Rgba8(200, 100, 0));

	Vec2 np_cap = GetNearestPointOnCapsule2D(m_aimP_pos, m_cap_start, m_cap_end, m_cap_radius);
	DebugDrawLine(np_cap, m_aimP_pos, 2, Rgba8(255, 255, 255, 30));
	DebugDrawCircle(5.f, np_cap, Rgba8(200, 100, 0));

	Vec2 np_aabb = GetNearestPointOnAABB2D(m_aimP_pos, AABB2(m_aabb_mins, m_aabb_maxs));
	DebugDrawLine(np_aabb, m_aimP_pos, 2, Rgba8(255, 255, 255, 30));
	DebugDrawCircle(5.f, np_aabb, Rgba8(200, 100, 0));

	Vec2 np_obb = GetNearestPointOnOBB2D(m_aimP_pos, OBB2(m_obb_center,m_obb_iBasisNormal,m_obb_halfDimensions));
	DebugDrawLine(np_obb, m_aimP_pos, 2, Rgba8(255, 255, 255, 30));
	DebugDrawCircle(5.f, np_obb, Rgba8(200, 100, 0));

	Vec2 np_lineseg = GetNearestPointOnLineSegment2D(m_aimP_pos, m_lineseg_start,m_lineseg_end);
	DebugDrawLine(np_lineseg, m_aimP_pos, 2, Rgba8(255, 255, 255, 30));
	DebugDrawCircle(5.f, np_lineseg, Rgba8(200, 100, 0));

	Vec2 np_infline = GetNearestPointOnInfiniteLine2D(m_aimP_pos, m_infline_start, m_infline_end);
	DebugDrawLine(np_infline, m_aimP_pos, 2, Rgba8(255, 255, 255, 30));
	DebugDrawCircle(5.f, np_infline, Rgba8(200, 100, 0));

	Vec2 np_triangle = GetNearestPointOnTriangle2D(m_aimP_pos, m_triangle);
	DebugDrawLine(np_triangle, m_aimP_pos, 2, Rgba8(255, 255, 255, 30));
	DebugDrawCircle(5.f, np_triangle, Rgba8(200, 100, 0));

	DebugDrawCircle(2.f, m_aimP_pos, Rgba8(255, 255, 255));

	std::vector<Vertex_PCU> title;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	font->AddVertsForTextInBox2D(title, "Mode (F6/F7 for prev/next): Nearest Point",AABB2(Vec2(10.f,770.f),Vec2(1600.f,790.f)),15.f,Rgba8(200,200,0),0.7f,Vec2(0.f,0.f));
	font->AddVertsForTextInBox2D(title, "F8 to randomize; LMB/RMB set ray start/end; ESDF move start, IJKL move end, arrows move ray,hold T=slow", AABB2(Vec2(10.f, 745.f), Vec2(1600.f, 765.f)), 15.f, Rgba8(0,200,200), 0.7f, Vec2(0.f, 0.f));
	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(title);

	g_theRenderer->EndCamera(m_screenCamera);
}

void GameNearestPoint::UpdateCamera(float deltaTime)
{
	UNUSED(deltaTime);
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
}

void GameNearestPoint::UpdateInput(float deltaTime)
{
	if (g_theInput->IsKeyDown('E'))
	{
		m_aimP_pos.y += AIMP_SPEED * deltaTime;
	}

	if (g_theInput->IsKeyDown('S'))
	{
		m_aimP_pos.x -= AIMP_SPEED * deltaTime;
	}

	if (g_theInput->IsKeyDown('D'))
	{
		m_aimP_pos.y -= AIMP_SPEED * deltaTime;
	}

	if (g_theInput->IsKeyDown('F'))
	{
		m_aimP_pos.x += AIMP_SPEED * deltaTime;
	}
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		Vec2 mousePos = g_theWindow->GetNormalizedMouseUV();
		AABB2 worldUV = AABB2(m_screenCamera.GetOrthoBottomLeft().x, m_screenCamera.GetOrthoBottomLeft().y,
			m_screenCamera.GetOrthoTopRight().x, m_screenCamera.GetOrthoTopRight().y);
		m_aimP_pos=worldUV.GetPointAtUV(mousePos);
	}
}

void GameNearestPoint::RenderBasicShape() const
{
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);

	std::vector<Vertex_PCU> disc_verts;
	AddVertsForDisc2D(disc_verts, m_disc_pos, m_disc_radius, m_disc_color);
	g_theRenderer->DrawVertexArray(disc_verts);

	std::vector<Vertex_PCU> cap_verts;
	AddVertsForCapsule2D(cap_verts, m_cap_start, m_cap_end, m_cap_radius, m_cap_color);
	g_theRenderer->DrawVertexArray(cap_verts);

	std::vector<Vertex_PCU> obb_verts;
	OBB2 obbBox = OBB2(m_obb_center, m_obb_iBasisNormal, m_obb_halfDimensions);
	AddVertsForOBB2D(obb_verts, obbBox, m_obb_color);
	g_theRenderer->DrawVertexArray(obb_verts);

	std::vector<Vertex_PCU> aabb_verts;
	AABB2 aabb_box = AABB2(m_aabb_mins, m_aabb_maxs);
	AddVertsForAABB2D(aabb_verts, aabb_box, m_aabb_color);
	g_theRenderer->DrawVertexArray(aabb_verts);

	std::vector<Vertex_PCU> lineseg_verts;
	AddVertsForLinSegment2D(lineseg_verts, m_lineseg_start, m_lineseg_end, 5.f, Rgba8(0, 100, 230, 255));
	g_theRenderer->DrawVertexArray(lineseg_verts);

	std::vector<Vertex_PCU> infline_verts;
	AddVertsForLinSegment2D(infline_verts, m_infline_start, m_infline_end, 5.f, Rgba8(0, 100, 230, 255));
	g_theRenderer->DrawVertexArray(infline_verts);

	std::vector<Vertex_PCU> triangle_verts;
	AddVertsForTriangle2D(triangle_verts, m_triangle, m_triangle_color);
	g_theRenderer->DrawVertexArray(triangle_verts);

}

void GameNearestPoint::SetColor(bool isBright, Rgba8& shapeColor)
{
	if (isBright)
	{
		shapeColor = BRIGHT_COLOR;
	}
	else
	{
		shapeColor = NORMAL_COLOR;
	}
}

void GameNearestPoint::ReRandomObject()
{
	//discs
	m_disc_pos = Vec2(m_rng.RollRandomFloatInRange(100.f, 1400.f),
		m_rng.RollRandomFloatInRange(50.f, 750.f));
	m_disc_radius = m_rng.RollRandomFloatInRange(20.f, 150.f);
	m_disc_color = NORMAL_COLOR;
	//cap
	m_cap_start = Vec2(m_rng.RollRandomFloatInRange(100.f, 1400.f),
		m_rng.RollRandomFloatInRange(50.f, 750.f));
	m_cap_end = Vec2(m_cap_start.x + m_rng.RollRandomFloatInRange(10.f, 100.f),
		m_cap_start.y + m_rng.RollRandomFloatInRange(10.f, 100.f));
	m_cap_radius = m_rng.RollRandomFloatInRange(10.f, 80.f);
	m_cap_color = NORMAL_COLOR;
	//obb2
	m_obb_center = Vec2(m_rng.RollRandomFloatInRange(100.f, 1400.f),
		m_rng.RollRandomFloatInRange(50.f, 750.f));
	m_obb_halfDimensions = Vec2(m_rng.RollRandomFloatInRange(50.f, 100.f),
		m_rng.RollRandomFloatInRange(50.f, 100.f));
	m_obb_iBasisNormal = Vec2::MakeFromPolarDegrees(m_rng.RollRandomFloatInRange(0.f, 360.f));
	m_obb_color = NORMAL_COLOR;
	//aabb
	m_aabb_mins = Vec2(m_rng.RollRandomFloatInRange(100.f, 1400.f),
		m_rng.RollRandomFloatInRange(50.f, 700.f));
	m_aabb_maxs = Vec2(m_rng.RollRandomFloatInRange(m_aabb_mins.x + 50.f, m_aabb_mins.x + 150.f),
		m_rng.RollRandomFloatInRange(m_aabb_mins.y + 50.f, m_aabb_mins.y + 150.f));
	m_aabb_color = NORMAL_COLOR;
	//line seg
	m_lineseg_start = Vec2(m_rng.RollRandomFloatInRange(100.f, 1400.f),
		m_rng.RollRandomFloatInRange(50.f, 700.f));
	m_lineseg_end = Vec2(m_rng.RollRandomFloatInRange(100.f, 1400.f),
		m_rng.RollRandomFloatInRange(50.f, 700.f));
	//inf line
	m_infline_start = Vec2(m_rng.RollRandomFloatInRange(-100.f, -50.f),
		m_rng.RollRandomFloatInRange(50.f, 700.f));
	m_infline_end = Vec2(m_rng.RollRandomFloatInRange(1700.f, 1800.f),
		m_rng.RollRandomFloatInRange(50.f, 700.f));
	//triangle
	Vec2 triangle_center = Vec2(m_rng.RollRandomFloatInRange(100.f, 1400.f),
		m_rng.RollRandomFloatInRange(50.f, 750.f));
	float triangle_radius = m_rng.RollRandomFloatInRange(50.f, 200.f);
	float triangle_degrees[3];
	triangle_degrees[0] = m_rng.RollRandomFloatInRange(0.f, 180.f);
	triangle_degrees[1] = triangle_degrees[0] + m_rng.RollRandomFloatInRange(10.f, 170.f);
	triangle_degrees[2] = triangle_degrees[1] + m_rng.RollRandomFloatInRange(10.f, 170.f);
	for (int i = 0; i < 3; i++)
	{
		m_triangle.m_pointsCounterClockwise[i] = triangle_center + Vec2::MakeFromPolarDegrees(triangle_degrees[i], triangle_radius);
	}
}
