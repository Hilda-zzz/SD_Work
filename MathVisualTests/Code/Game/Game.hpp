#pragma once
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"

class Clock;
class RaycastArrow;

enum GameMode
{
	GAME_MODE_NEAREST_POINT,
	GAME_MODE_RAYCAST_VS_DISCS,
	GAME_MODE_RAYCAST_VS_LINESEG,
	GAME_MODEE_RAYCAST_VS_AABB2D,
	GAME_MODE_3DSHAPES,
	COUNT
};

class Game
{
public:
	Game();
	virtual ~Game() = default;

	virtual void Update()=0;
	virtual void Renderer() const=0;
	virtual void UpdateCamera(float deltaTime)=0;

protected:
	void UpdateDeveloperCheats(float& deltaTime);
	void UpdateKBInputForRaycastMode(float& deltaTime,RaycastArrow& arrow,float speed);
	void AdjustForPauseAndTimeDitortion(float& deltaSeconds);

public:
	RandomNumberGenerator m_rng;

protected:
	bool m_isPause = false;
	bool m_isSlow = false;
	bool m_pauseAfterUpdate = false;
};