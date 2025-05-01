#pragma once
#include "Game/Game.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Triangle2.hpp"
#include <Engine/Math/CubicBezierCurve2D.hpp>
#include <Engine/Core/Timer.hpp>
#include <Engine/Math/CubicHermiteSpline2D.hpp>

class Clock;

class GameCurve :public Game
{
public:
	GameCurve();
	~GameCurve();
	void Update() override;
	void Renderer() const override;
	void UpdateCamera(float deltaTime) override;

	void GenerateBezierCurve(bool ifRandom=true);
	void GenerateHermiteSpline(bool ifRandom = true);

	Clock* m_gameClock = nullptr;
private:
	void UpdateInput(float deltaTime);
	void ReRandomObject();
	
	Camera m_screenCamera;

	bool m_isDebugDraw = false;

	std::vector<Vertex_PCU> m_easingVerts;
	std::vector<Vertex_PCU> m_bezierVerts;
	std::vector<Vertex_PCU> m_movePointsVerts;
	std::vector<Vertex_PCU> m_splineVerts;

	int m_subdivisionCount = 64;

	size_t m_easingFuncIndex = 0;

	CubicBezierCurve2D m_bezierCurve;
	Timer m_2SecTimer;
	float m_curBezierLen;

	int accumlationTimes = 1;

	CubicHermiteSpline2D m_hermiteSpline;
};