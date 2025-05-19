#include "GameCurve.hpp"
#include "Engine/Core/Clock.hpp"
#include "App.hpp"
#include "Engine/Window/Window.hpp"
#include <Engine/Core/VertexUtils.hpp>
#include <Engine/Math/Easing.hpp>
#include <Engine/Math/MathUtils.hpp>
#include "Engine/Math/CubicBezierCurve2D.hpp"

struct EasingFunc;



GameCurve::GameCurve():m_bezierCurve(CubicBezierCurve2D(Vec2::ZERO, Vec2::ZERO, Vec2::ZERO, Vec2::ZERO))
{
	g_theInput->SetCursorMode(CursorMode::POINTER);
	g_theWindow->SetCursorVisible(true);

	m_gameClock = new Clock();

	m_movePointsVerts.reserve(1000);
	ReRandomObject();

	m_2SecTimer = Timer(2.f, m_gameClock);
	m_2SecTimer.Start();
}

GameCurve::~GameCurve()
{
	m_gameClock->m_parent->RemoveChild(m_gameClock);
	delete m_gameClock;
	m_gameClock = nullptr;
}

void GameCurve::Update()
{
	if (g_theInput->IsKeyDown('T'))
	{
		m_gameClock->SetTimeScale(0.1f);
	}
	else
	{
		m_gameClock->SetTimeScale(1.f);
	}
	float deltaTime = (float)m_gameClock->GetDeltaSeconds();

	if (m_2SecTimer.GetElapsedTime() >= m_2SecTimer.m_period)
	{
		m_2SecTimer.Stop();
		m_2SecTimer.Start();
		accumlationTimes++;
	}
	if (accumlationTimes > (int)m_hermiteSpline.m_curves.size())
	{
		accumlationTimes = 1;
	}

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
	UpdateCamera(deltaTime);
	//---------------------------------------------
	// Easing
	m_easingVerts.reserve((m_subdivisionCount+64) * 6);
	m_easingVerts.clear();

	float stepHigh = 1.f / 64;
	for (int i = 0; i < 64; i++)
	{
		float t = stepHigh * i;
		float nextT = t + stepHigh;

		Vec2 Start = Vec2(
			RangeMap(t, 0.f, 1.f, 260.f, 560.f),
			RangeMap(g_easingFunctions[m_easingFuncIndex].m_funcPointer(t), 0.f, 1.f, 430.f, 730.f));

		Vec2 End = Vec2(
			RangeMap(nextT, 0.f, 1.f, 260.f, 560.f),
			RangeMap(g_easingFunctions[m_easingFuncIndex].m_funcPointer(nextT), 0.f, 1.f, 430.f, 730.f));

		AddVertsForLinSegment2D(m_easingVerts, Start, End, 2.f, Rgba8(90, 90, 90));
	}

	float step = 1.f / m_subdivisionCount;
	for (int i = 0; i < m_subdivisionCount; i++)
	{
		float t = step * i;
		float nextT = t + step;

		Vec2 Start = Vec2( 
			RangeMap(t,0.f,1.f,260.f,560.f), 
			RangeMap(g_easingFunctions[m_easingFuncIndex].m_funcPointer(t), 0.f, 1.f, 430.f, 730.f));

		Vec2 End = Vec2(
			RangeMap(nextT, 0.f, 1.f, 260.f, 560.f), 
			RangeMap(g_easingFunctions[m_easingFuncIndex].m_funcPointer(nextT), 0.f, 1.f, 430.f, 730.f));

		AddVertsForLinSegment2D(m_easingVerts, Start, End, 2.f, Rgba8::GREEN);
	}
	//---------------------------------------------
	// Moving Point
	float curTimeBezierDistForT = ((float)m_2SecTimer.GetElapsedTime()/ (float)m_2SecTimer.m_period)*m_curBezierLen;
	Vec2 bezierVPoint = m_bezierCurve.EvaluateAtApproximateDistance(curTimeBezierDistForT);

	float curTimeRatio = (float)m_2SecTimer.GetElapsedTime()/ (float)m_2SecTimer.m_period;

	Vec2 easingTPoint = Vec2(
			RangeMap(curTimeRatio, 0.f, 1.f, 260.f, 560.f),
			RangeMap(g_easingFunctions[m_easingFuncIndex].m_funcPointer(curTimeRatio), 0.f, 1.f, 430.f, 730.f));
	Vec2 bezierTPoint = m_bezierCurve.EvaluateAtParametric(curTimeRatio);
	
	m_movePointsVerts.clear();
 	AddVertsForLinSegment2D(m_movePointsVerts, easingTPoint, Vec2(260.f, easingTPoint.y), 1.5f, Rgba8(152, 161, 242,100));
 	AddVertsForLinSegment2D(m_movePointsVerts, easingTPoint, Vec2(easingTPoint.x, 430.f), 1.5f, Rgba8(152, 161, 242, 100));
 	AddVertsForDisc2D(m_movePointsVerts, easingTPoint, 5.f, Rgba8::WHITE);
 	AddVertsForDisc2D(m_movePointsVerts, bezierTPoint,5.f, Rgba8::WHITE);
	AddVertsForDisc2D(m_movePointsVerts, bezierVPoint, 5.f, Rgba8::GREEN);
	//---------------------------------------------
	// Spline Moving Point
	float splineCurTimeRatio = ((accumlationTimes - 1) * (float)m_2SecTimer.m_period + (float)m_2SecTimer.GetElapsedTime()) /
		(m_hermiteSpline.m_curves.size() * (float)m_2SecTimer.m_period);
	Vec2 splineTPoint = m_hermiteSpline.EvaluateAtParametric(splineCurTimeRatio);
	AddVertsForDisc2D(m_movePointsVerts, splineTPoint, 5.f, Rgba8::WHITE);

	float curTimeSplineDistForT = splineCurTimeRatio * m_hermiteSpline.GetApproximateLength(m_subdivisionCount);
	Vec2 splineVPoint = m_hermiteSpline.EvaluateAtApproximateDistance(curTimeSplineDistForT);
	AddVertsForDisc2D(m_movePointsVerts, splineVPoint, 5.f, Rgba8::GREEN);
}

void GameCurve::Renderer() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	//-----------draw title-------------------
	g_theRenderer->SetModelConstants();
	std::vector<Vertex_PCU> title;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	font->AddVertsForTextInBox2D(title, "Mode (F6/F7 for prev/next): Curves, Splines, Easing(2D)", AABB2(Vec2(10.f, 770.f), Vec2(1600.f, 790.f)), 15.f, Rgba8(200, 200, 0), 0.7f, Vec2(0.f, 0.f));

	std::string message = "F8 to randomize; W/E = prev/next Easing functions; N/M = curve subdivisions ("
		+ std::to_string(m_subdivisionCount) + "), hold T=slow";
	font->AddVertsForTextInBox2D(title, message, AABB2(Vec2(10.f, 745.f), Vec2(1600.f, 765.f)), 15.f, Rgba8(0, 200, 200), 0.7f, Vec2(0.f, 0.f));
	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(title);
	//----------draw box----------------------
	std::vector<Vertex_PCU> boxVerts;
	if (m_isDebugDraw)
	{
		AABB2 topLeftBox = AABB2(Vec2(30.f, 395.f), Vec2(790.f, 730.f));
		AABB2 topRightBox = AABB2(Vec2(810.f, 395.f), Vec2(1570.f, 730.f));
		AABB2 botBox = AABB2(Vec2(30.f, 20.f), Vec2(1570.f, 375.f));
		AddVertsForAABB2D(boxVerts, topLeftBox, Rgba8(30, 30, 30));
		AddVertsForAABB2D(boxVerts, topRightBox, Rgba8(30, 30, 30));
		AddVertsForAABB2D(boxVerts, botBox, Rgba8(30, 30, 30));
	}
	AABB2 easingCurveBox = AABB2(Vec2(30.f, 395.f)+Vec2(230.f,35.f), Vec2(790.f, 730.f)- Vec2(230.f, 0.f));
	AddVertsForAABB2D(boxVerts, easingCurveBox, Rgba8(35,35,80));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(boxVerts);

	std::vector<Vertex_PCU> EasingTitle;
	font->AddVertsForTextInBox2D(EasingTitle,g_easingFunctions[m_easingFuncIndex].m_funcName, AABB2(Vec2(260.f, 395.f), Vec2(560.f, 430.f)), 20.f, Rgba8(0, 200, 200), 0.7f, Vec2(0.5f, 0.5f));
	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(EasingTitle);
	//----------draw easing curve----------------------
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(m_easingVerts);
	//----------draw bezier curve----------------------
	g_theRenderer->DrawVertexArray(m_bezierVerts);
	//----------draw spline----------------------
	g_theRenderer->DrawVertexArray(m_splineVerts);

	g_theRenderer->DrawVertexArray(m_movePointsVerts);

	g_theRenderer->EndCamera(m_screenCamera);
}

void GameCurve::UpdateCamera(float deltaTime)
{
	UNUSED(deltaTime);

	IntVec2 dimensions = g_theWindow->GetClientDimensions();

	m_screenCamera.SetViewport(AABB2(Vec2::ZERO, Vec2((float)dimensions.x, (float)dimensions.y)));
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));

}

void GameCurve::GenerateBezierCurve(bool ifRandom)
{
	m_bezierVerts.clear();
	m_bezierVerts.reserve(m_subdivisionCount * 6+500);
	
	Vec2 startPoint;
	Vec2 guide1Point;
	Vec2 guide2Point;
	Vec2 endPoint;
	if (ifRandom)
	{
		startPoint = Vec2(m_rng.RollRandomFloatInRange(810.f, 1570.f), m_rng.RollRandomFloatInRange(395.f, 730.f));
		guide1Point = Vec2(m_rng.RollRandomFloatInRange(810.f, 1570.f), m_rng.RollRandomFloatInRange(395.f, 730.f));
		guide2Point = Vec2(m_rng.RollRandomFloatInRange(810.f, 1570.f), m_rng.RollRandomFloatInRange(395.f, 730.f));
		endPoint = Vec2(m_rng.RollRandomFloatInRange(810.f, 1570.f), m_rng.RollRandomFloatInRange(395.f, 730.f));
		m_bezierCurve = CubicBezierCurve2D(startPoint, guide1Point, guide2Point, endPoint);
	}
	else
	{
		startPoint = m_bezierCurve.m_startPos;
		guide1Point = m_bezierCurve.m_guidePos1;
		guide2Point = m_bezierCurve.m_guidePos1;
		endPoint = m_bezierCurve.m_endPos;
	}


	AddVertsForLinSegment2D(m_bezierVerts, startPoint, guide1Point, 1.5f, Rgba8(35, 35, 80));
	AddVertsForLinSegment2D(m_bezierVerts, guide1Point, guide2Point, 1.5f, Rgba8(35, 35, 80));
	AddVertsForLinSegment2D(m_bezierVerts, guide2Point, endPoint, 1.5f, Rgba8(35, 35, 80));

	float highStep = 1.f / 64;
	for (int i = 0; i < 64; i++)
	{
		float t = highStep * i;
		float nextT = t + highStep;

		Vec2 Start = m_bezierCurve.EvaluateAtParametric(t);

		Vec2 End = m_bezierCurve.EvaluateAtParametric(nextT);

		AddVertsForLinSegment2D(m_bezierVerts, Start, End, 2.f, Rgba8(90,90,90));
	}

	float step = 1.f / m_subdivisionCount;
	for (int i = 0; i < m_subdivisionCount; i++)
	{
		float t = step * i;
		float nextT = t + step;

		Vec2 Start = m_bezierCurve.EvaluateAtParametric(t);

		Vec2 End = m_bezierCurve.EvaluateAtParametric(nextT);

		AddVertsForLinSegment2D(m_bezierVerts, Start, End, 2.f, Rgba8::GREEN);
	}

	AddVertsForDisc2D(m_bezierVerts, startPoint, 4.f, Rgba8::HILDA);
	AddVertsForDisc2D(m_bezierVerts, guide1Point, 4.f, Rgba8::HILDA);
	AddVertsForDisc2D(m_bezierVerts, guide2Point, 4.f, Rgba8::HILDA);
	AddVertsForDisc2D(m_bezierVerts, endPoint, 4.f, Rgba8::HILDA);

	m_curBezierLen = m_bezierCurve.GetApproximateLength(64);
}

void GameCurve::GenerateHermiteSpline(bool ifRandom)
{
	m_splineVerts.clear();

	if (ifRandom)
	{
		std::vector<Vec2> allControlPoints;

		int controlPointsCount = m_rng.RollRandomIntInRange(3, 9);
		allControlPoints.reserve(controlPointsCount);
		m_splineVerts.reserve(m_subdivisionCount * 6 * (controlPointsCount - 1) + 1000);
		float xStep = 1540.f / (controlPointsCount + 1);
		float curXRangeCenter = 30.f;

		for (int i = 0; i < controlPointsCount; i++)
		{
			curXRangeCenter += xStep;
			float x = m_rng.RollRandomFloatInRange(curXRangeCenter - 10.f, curXRangeCenter + 10.f);
			float y = m_rng.RollRandomFloatInRange(30.f, 365.f);
			allControlPoints.push_back(Vec2(x, y));
		}
		m_hermiteSpline = CubicHermiteSpline2D(allControlPoints);
	}
	
	//control point line
	for (int i = 0; i < (int)m_hermiteSpline.m_curves.size(); i++)
	{
		AddVertsForLinSegment2D(m_splineVerts, m_hermiteSpline.m_points[i], m_hermiteSpline.m_points[i + 1], 1.5f, Rgba8(35, 35, 80));
	}

	//high spline
	int totalHighSubCount = 64 * (int)m_hermiteSpline.m_curves.size();
	float highStep = 1.f / totalHighSubCount;
	for (int i = 0; i < totalHighSubCount - 1; i++)
	{
		float t = highStep * i;
		float nextT = t + highStep;

		Vec2 Start = m_hermiteSpline.EvaluateAtParametric(t);

		Vec2 End = m_hermiteSpline.EvaluateAtParametric(nextT);

		AddVertsForLinSegment2D(m_splineVerts, Start, End, 2.f, Rgba8(90, 90, 90));
	}

	//cur sub spline
	int totalSubCount = m_subdivisionCount * (int)m_hermiteSpline.m_curves.size();
	float step = 1.f / totalSubCount;
	for (int i = 0; i < totalSubCount; i++)
	{
		float t = step * i;
		float nextT = t + step;

		Vec2 Start = m_hermiteSpline.EvaluateAtParametric(t);

		Vec2 End = m_hermiteSpline.EvaluateAtParametric(nextT);

		AddVertsForLinSegment2D(m_splineVerts, Start, End, 2.f, Rgba8::GREEN);
	}

	//velocity arrow
	for (int i = 1; i < (int)m_hermiteSpline.m_curves.size(); i++)
	{
		AddVertsForArrow2D(m_splineVerts,
			m_hermiteSpline.m_points[i], m_hermiteSpline.m_points[i] + m_hermiteSpline.m_velocities[i],
			8.f, 1.5f, Rgba8::MAGNETA);
	}

	//control point
	for (int i = 0; i < (int)m_hermiteSpline.m_points.size(); i++)
	{
		AddVertsForDisc2D(m_splineVerts, m_hermiteSpline.m_points[i], 4.f, Rgba8::HILDA);
	}
}

void GameCurve::UpdateInput(float deltaTime)
{
	UNUSED(deltaTime);
	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebugDraw = !m_isDebugDraw;
	}

	if (g_theInput->WasKeyJustPressed('W'))
	{
		if (m_easingFuncIndex == 0)
		{
			m_easingFuncIndex = g_numEasingFunctions - 1;
		}
		else
		{
			m_easingFuncIndex--;
		}
	}

	if (g_theInput->WasKeyJustPressed('E'))
	{
		m_easingFuncIndex = (m_easingFuncIndex + 1) % (g_numEasingFunctions);
	}

	if (g_theInput->WasKeyJustPressed('N'))
	{
		if (m_subdivisionCount > 1)
		{
			m_subdivisionCount /= 2;
		}
		GenerateBezierCurve(false);
		GenerateHermiteSpline(false);
	}
	if (g_theInput->WasKeyJustPressed('M'))
	{
		m_subdivisionCount *= 2;
		GenerateBezierCurve(false);
		GenerateHermiteSpline(false);
	}
}

void GameCurve::ReRandomObject()
{
	GenerateBezierCurve();
	GenerateHermiteSpline();
}
