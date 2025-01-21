#pragma once
#include "Game/Game.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "RaycastAtrrow.hpp"
#include "Engine/Math/AABB2.hpp"
#include "DiscObstacle.hpp"
class GameRaycastVsDiscs :public Game
{
public:
	GameRaycastVsDiscs();
	~GameRaycastVsDiscs();
	void Update(float deltaTime) override;
	void UpdateKeyboardInput(float deltaTime);
	void Renderer() const override;
	void RerandomDiscs();
	void UpdateCamera(float deltaTime) override;
private:
	Camera m_screenCamera;
	RaycastResult2D m_final_result;
	RaycastArrow m_arrow;
	AABB2 m_worldUV;
	std::vector<DiscObstacle*> m_discs;
	int m_index = -1;
};