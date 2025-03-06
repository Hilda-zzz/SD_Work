#pragma once
#include "Game/Game.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "RaycastAtrrow.hpp"
#include "Engine/Math/AABB2.hpp"

class LineSegObstacle;

class GameRaycastVsLineSeg :public Game
{
public:
	GameRaycastVsLineSeg();
	~GameRaycastVsLineSeg();
	void Update() override;
	void Renderer() const override;
	void RerandomLineSegs();
	void UpdateCamera(float deltaTime) override;
private:
	Clock* m_gameClock = nullptr;
	Camera m_screenCamera;
	RaycastResult2D m_final_result;
	RaycastArrow m_arrow;
	AABB2 m_worldUV;
	std::vector<LineSegObstacle*> m_lineSegs;
	int m_index = -1;
};