#pragma once
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"

enum GameMode
{
	GAME_MODE_NEAREST_POINT,
	GAME_MODE_RAYCAST_VS_DISCS
};

class Game
{
public:
	Game();
	virtual ~Game() = default;

	virtual void Update(float deltaSeconds)=0;
	virtual void Renderer() const=0;
	virtual void UpdateCamera(float deltaTime)=0;

protected:
	void UpdateDeveloperCheats(float& deltaTime);
	void AdjustForPauseAndTimeDitortion(float& deltaSeconds);

public:
	RandomNumberGenerator m_rng;

protected:
	bool m_isPause = false;
	bool m_isSlow = false;
	bool m_pauseAfterUpdate = false;
};