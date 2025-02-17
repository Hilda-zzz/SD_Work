#pragma once
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
class Game
{
public:
	Game();
	~Game();

	void Update(float deltaSeconds);
	void Renderer() const;

private:
	void UpdateAttractMode(float deltaTime);
	void UpdateGameplayMode(float deltaTime);
	void UpdateDeveloperCheats(float deltaTime);
	void UpdateCamera(float deltaTime);
	void AdjustForPauseAndTimeDitortion(float& deltaSeconds);
	void RenderAttractMode() const;
	void RenderUI() const;
	void RenderDebugMode() const;

public:
	bool m_isAttractMode = true;
	bool isDebugMode = false;
	RandomNumberGenerator m_rng;

private:
	Camera m_screenCamera;
	bool m_isPause = false;
	bool m_isSlow = false;
	bool m_pauseAfterUpdate = false;
};