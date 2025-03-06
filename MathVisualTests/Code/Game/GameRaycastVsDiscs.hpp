#pragma once
#include "Game/Game.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "RaycastAtrrow.hpp"
#include "Engine/Math/AABB2.hpp"
#include "DiscObstacle.hpp"

class Clock;

class GameRaycastVsDiscs :public Game
{
public:
	GameRaycastVsDiscs();
	~GameRaycastVsDiscs();
	void Update() override;
	void Renderer() const override;
	void RerandomDiscs();
	void UpdateCamera(float deltaTime) override;

	Clock* m_gameClock = nullptr;

private:
	Camera m_screenCamera;
	RaycastResult2D m_final_result;
	RaycastArrow m_arrow;
	AABB2 m_worldUV;
	std::vector<DiscObstacle*> m_discs;
	int m_index = -1;
};