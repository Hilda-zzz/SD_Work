//#pragma once
//#include "BasePlayer.hpp"
//#include "Engine/Math/Vec2.hpp"
//#include "Engine/Core/Rgba8.hpp"
//#include "Engine/Math/MathUtils.hpp"
//#include "CellMatDef.hpp"
//
//class GameMap;
//
//class GamePlayer : public BasePlayer {
//public:
//	GamePlayer(IntVec2 const& mapSize, Vec2 const& startPosition);
//	~GamePlayer() override = default;
//
//	// === BasePlayer Interface ===
//	void Update(float deltaTime) override;
//	void HandleInput() override;
//	void RenderImgui() override;
//
//	void InitCamera(IntVec2 const& mapSize) override;
//
//	// === Player Movement ===
//	void SetPosition(Vec2 const& position);
//	Vec2 GetPosition() const { return m_position; }
//
//	void SetVelocity(Vec2 const& velocity) { m_velocity = velocity; }
//	Vec2 GetVelocity() const { return m_velocity; }
//
//	// === Rendering ===
//	void Render() const;
//
//	// === Configuration ===
//	void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
//	void SetRadius(float radius) { m_radius = radius; }
//	void SetColor(Rgba8 const& color) { m_color = color; }
//
//	float GetRadius() const { return m_radius; }
//	Rgba8 GetColor() const { return m_color; }
//
//	// === Camera Zoom ===
//	void SetCameraZoom(float zoom);
//	float GetCameraZoom() const { return m_cameraZoom; }
//	void SetZoomLimits(float minZoom, float maxZoom);
//
//	// === Map Reference ===
//	GameMap* GetGameMap() { return m_gameMap; }
//	void SetGameMap(GameMap* map) { m_gameMap = map; }
//
//	AABB2 const& GetBaseCamBound() const;
//
//	CellMatType GetSelectedMaterial() const { return m_selectedMaterial; }
//	int GetBrushSize() const { return m_brushSize; }
//	void SetSelectedMaterial(CellMatType material) {
//		m_selectedMaterial = material;
//	}
//
//	void SetBrushSize(int size) {
//		m_brushSize = GetClamped(size, 1, 20);
//	}
//
//private:
//	void UpdateMovement(float deltaTime);
//	void UpdateCamera();
//	void HandleMovementInput();
//	void HandleCameraZoomInput();
//	void HandleMatBrushInput();
//private:
//	// Position & Physics
//	Vec2 m_position;              // World position
//	Vec2 m_velocity;              // Current velocity
//
//	// Movement
//	float m_moveSpeed;            // Movement speed (cells per second)
//	float m_acceleration;         // Acceleration rate
//	float m_deceleration;         // Deceleration rate
//
//	// Visual
//	float m_radius;               // Circle radius for rendering
//	Rgba8 m_color;                // Player color
//
//	// Camera Zoom
//	float m_cameraZoom;           // Current zoom level (1.0 = default)
//	float m_minZoom;              // Minimum zoom (zoom in limit)
//	float m_maxZoom;              // Maximum zoom (zoom out limit)
//	float m_baseViewWidth;        // Base view width (at zoom 1.0)
//	float m_baseViewHeight;       // Base view height (at zoom 1.0)
//	float m_zoomSpeed;            // Zoom speed multiplier
//
//	// References
//	GameMap* m_gameMap;
//
//	// Input
//	CellMatType m_selectedMaterial = CellMatType::MAT_SAND;
//	int m_brushSize = 1;
//	IntVec2 m_lastPlacedPos = IntVec2(-1, -1);
//};

#pragma once
#include "BasePlayer.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "CellMatDef.hpp"
#include <ThirdParty/box2d/include/box2d/box2d.h>

class GameMap;

// ========================================
// Player Animation State
// ========================================
enum class PlayerAnimState
{
	IDLE,
	MOVE,
};

// ========================================
// GamePlayer
// ========================================
class GamePlayer : public BasePlayer {
public:
	GamePlayer(IntVec2 const& mapSize, Vec2 const& startPosition);
	~GamePlayer() override;

	// === BasePlayer Interface ===
	void Update(float deltaTime) override;
	void HandleInput() override;
	void RenderImgui() override;
	void InitCamera(IntVec2 const& mapSize) override;

	// === Physics-aware split update (call these from GameMap::Update) ===
	// 1. Call PrePhysicsUpdate  – processes input, pushes velocity into Box2D
	// 2. Step the Box2D world   – done by GameMap::UpdatePhysicsWorld
	// 3. Call PostPhysicsUpdate – pulls resolved position, updates anim & camera
	void PrePhysicsUpdate(float deltaTime);
	void PostPhysicsUpdate(float deltaTime);

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

	// === Camera Smoothing ===
	void SetCameraSmoothing(bool enabled) { m_cameraSmoothing = enabled; }
	bool GetCameraSmoothing() const { return m_cameraSmoothing; }
	void SetCameraSmoothSpeed(float speed) { m_cameraSmoothSpeed = speed; }

	// === Map Reference ===
	GameMap* GetGameMap() { return m_gameMap; }
	void SetGameMap(GameMap* map);

	AABB2 const& GetBaseCamBound() const;

	// === Material Brush ===
	CellMatType GetSelectedMaterial() const { return m_selectedMaterial; }
	int GetBrushSize() const { return m_brushSize; }
	void SetSelectedMaterial(CellMatType material) { m_selectedMaterial = material; }
	void SetBrushSize(int size) { m_brushSize = GetClamped(size, 1, 20); }

	// === Physics ===
	void SetPhysicsEnabled(bool enabled);
	bool IsPhysicsEnabled() const { return m_physicsEnabled; }

	// === Sprite Animation ===
	// Call this after construction to provide sprite sheet assets.
	// sheet      : the sprite sheet to use (caller owns memory)
	// idleRow    : row index (0-based) of the idle animation frames
	// moveRow    : row index (0-based) of the move animation frames
	// frameCount : number of frames per animation (same for both)
	// fps        : frames per second for both animations
	void InitSprite(Texture* texture, IntVec2 const& gridLayout,
		int idleStartIndex, int idleEndIndex,
		int moveStartIndex, int moveEndIndex,
		float fps);
	bool HasSprite() const { return m_spriteSheet != nullptr; }

private:
	// === Update Helpers ===
	void UpdateMovement(float deltaTime);
	void UpdateCamera(float deltaTime);
	void UpdateAnimation(float deltaTime);

	// === Input Helpers ===
	void HandleMovementInput();
	void HandleCameraZoomInput();
	void HandleMatBrushInput();

	// === Physics Helpers ===
	void CreatePlayerBody();
	void DestroyPlayerBody();
	void SyncBodyToPosition();   // push position into Box2D
	void SyncPositionFromBody(); // pull position from Box2D after step

private:
	// ── Position & Physics ──────────────────────────────────────────────
	Vec2  m_position;
	Vec2  m_velocity;

	// ── Movement ────────────────────────────────────────────────────────
	float m_moveSpeed;
	float m_acceleration;
	float m_deceleration;

	// ── Visual (fallback circle) ─────────────────────────────────────────
	float m_radius;
	Rgba8 m_color;

	// ── Camera Zoom ──────────────────────────────────────────────────────
	float m_cameraZoom;
	float m_minZoom;
	float m_maxZoom;
	float m_baseViewWidth;
	float m_baseViewHeight;
	float m_zoomSpeed;

	// ── Camera Smoothing ─────────────────────────────────────────────────
	bool  m_cameraSmoothing = true;
	float m_cameraSmoothSpeed = 8.0f;   // lerp factor (units/sec feel)
	Vec2  m_cameraCurrentPos;            // smoothed camera centre

	// ── Map Reference ────────────────────────────────────────────────────
	GameMap* m_gameMap;

	// ── Material Brush ───────────────────────────────────────────────────
	CellMatType m_selectedMaterial = CellMatType::MAT_SAND;
	int         m_brushSize = 1;
	IntVec2     m_lastPlacedPos = IntVec2(-1, -1);

	// ── Box2D Kinematic Body ─────────────────────────────────────────────
	b2BodyId m_b2BodyId = b2_nullBodyId;
	bool     m_physicsEnabled = true;

	// ── Sprite Animation ─────────────────────────────────────────────────
	SpriteSheet* m_spriteSheet = nullptr;   // not owned
	SpriteAnimDefinition* m_idleAnim = nullptr;
	SpriteAnimDefinition* m_moveAnim = nullptr;

	PlayerAnimState m_animState = PlayerAnimState::IDLE;
	float           m_animTimer = 0.0f;
	bool            m_facingRight = true;             // for horizontal flip
};