#pragma once
#include "Game/Game.hpp"
#include <vector>
#include <ThirdParty/box2d/include/box2d/id.h>


class Clock;

class GameBox2DTest : public Game
{
public:
	GameBox2DTest();
	~GameBox2DTest();

	void Update() override;
	void Renderer() const override;
	void UpdateCamera(float deltaTime) override;

private:
	void UpdateInput(float deltaTime);
	void UpdatePhysics(float fixedTimeStep);

	void HandleMouseInput();
	b2BodyId GetBodyAtMousePosition(const Vec2& worldPos);
	Vec2 ScreenToWorld(const Vec2& screenPos) const;

private:
	Clock* m_gameClock = nullptr;
	Camera m_screenCamera;
	AABB2 m_worldUV;


	b2WorldId m_worldId;
	b2BodyId m_groundBodyId;
	std::vector<b2BodyId> m_dynamicBoxes;

	b2BodyId m_selectedBodyId;
	bool m_isDragging = false;
	Vec2 m_dragOffset;  // mouse related to box center

	float m_physicsAccumulator = 0.f;
	float m_fixedTimeStep = 1.0f / 60.0f;  // 60 FPS
	float m_curDeltaTime = 0.f;

	mutable std::vector<Vertex_PCU> m_worldVerts;

	static constexpr float SCREEN_SIZE_X = 1600.f;
	static constexpr float SCREEN_SIZE_Y = 800.f;

	// pixel to meter
	static constexpr float PIXELS_PER_METER = 50.f;
	static constexpr float METERS_PER_PIXEL = 1.f / PIXELS_PER_METER;

	static constexpr float DRAG_STIFFNESS = 50.f;
	static constexpr float DRAG_DAMPING = 10.f;
};