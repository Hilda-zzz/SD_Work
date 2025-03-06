#pragma once
#include "Game/Game.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "RaycastAtrrow.hpp"
#include "Engine/Math/AABB2.hpp"

class AABBObstacle;

class GameRaycastVsAABB2D :public Game
{
public:
	GameRaycastVsAABB2D();
	~GameRaycastVsAABB2D();
	void Update() override;
	void Renderer() const override;
	void RerandomBoxs();
	void UpdateCamera(float deltaTime) override;
	AABBObstacle* GenerateEachBox();
private:
	Clock* m_gameClock = nullptr;
	Camera m_screenCamera;
	RaycastResult2D m_final_result;
	RaycastArrow m_arrow;
	AABB2 m_worldUV;
	std::vector<AABBObstacle*> m_boxs;
	int m_index = -1;
};