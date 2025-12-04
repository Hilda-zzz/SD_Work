#pragma once
#include "BasePlayer.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

class GameMap;

class GamePlayer : public BasePlayer {
public:
	GamePlayer(IntVec2 const& mapSize, Vec2 const& startPosition);
	~GamePlayer() override = default;

	// === BasePlayer Interface ===
	void Update(float deltaTime) override;
	void HandleInput() override;
	void RenderImgui() override;

	void InitCamera(IntVec2 const& mapSize) override;

	// === Player Movement ===
	void SetPosition(Vec2 const& position);
	Vec2 GetPosition() const { return m_position; }

	void SetVelocity(Vec2 const& velocity) { m_velocity = velocity; }
	Vec2 GetVelocity() const { return m_velocity; }

	// === Rendering ===
	void Render() const;

	// === Configuration ===
	void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
	void SetRadius(float radius) { m_radius = radius; }
	void SetColor(Rgba8 const& color) { m_color = color; }

	float GetRadius() const { return m_radius; }
	Rgba8 GetColor() const { return m_color; }

	// === Camera Zoom ===
	void SetCameraZoom(float zoom);
	float GetCameraZoom() const { return m_cameraZoom; }
	void SetZoomLimits(float minZoom, float maxZoom);

	// === Map Reference ===
	GameMap* GetGameMap() { return m_gameMap; }
	void SetGameMap(GameMap* map) { m_gameMap = map; }

	AABB2 const& GetBaseCamBound() const;

private:
	void UpdateMovement(float deltaTime);
	void UpdateCamera();
	void HandleMovementInput();
	void HandleCameraZoomInput();

private:
	// Position & Physics
	Vec2 m_position;              // World position
	Vec2 m_velocity;              // Current velocity

	// Movement
	float m_moveSpeed;            // Movement speed (cells per second)
	float m_acceleration;         // Acceleration rate
	float m_deceleration;         // Deceleration rate

	// Visual
	float m_radius;               // Circle radius for rendering
	Rgba8 m_color;                // Player color

	// Camera Zoom
	float m_cameraZoom;           // Current zoom level (1.0 = default)
	float m_minZoom;              // Minimum zoom (zoom in limit)
	float m_maxZoom;              // Maximum zoom (zoom out limit)
	float m_baseViewWidth;        // Base view width (at zoom 1.0)
	float m_baseViewHeight;       // Base view height (at zoom 1.0)
	float m_zoomSpeed;            // Zoom speed multiplier

	// References
	GameMap* m_gameMap;
};