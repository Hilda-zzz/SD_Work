//#include "GamePlayer.hpp"
//#include "GameMap.hpp"
//#include "Engine/Input/InputSystem.hpp"
//#include "Engine/Window/Window.hpp"
//#include "Engine/Math/MathUtils.hpp"
//#include "Engine/Core/VertexUtils.hpp"
//#include "Engine/Renderer/Renderer.hpp"
//
//extern InputSystem* g_theInput;
//extern Window* g_theWindow;
//extern Renderer* g_theRenderer;
//
//GamePlayer::GamePlayer(IntVec2 const& mapSize, Vec2 const& startPosition)
//	: //BasePlayer(mapSize)
//	 m_position(startPosition)
//	, m_velocity(Vec2::ZERO)
//	, m_moveSpeed(200.0f)
//	, m_acceleration(1000.0f)
//	, m_deceleration(800.0f)
//	, m_radius(13.0f)
//	, m_color(Rgba8(255, 200, 0))
//	, m_cameraZoom(1.0f)
//	, m_minZoom(0.05f)
//	, m_maxZoom(5.0f)
//	, m_zoomSpeed(0.1f)
//	, m_gameMap(nullptr)
//{
//
//}
//
//void GamePlayer::Update(float deltaTime)
//{
//	HandleInput();
//	UpdateMovement(deltaTime);
//	UpdateCamera();
//}
//
//void GamePlayer::HandleInput()
//{
//	HandleMovementInput();
//	HandleCameraZoomInput();
//	HandleMatBrushInput();
//}
//
//void GamePlayer::HandleMovementInput()
//{
//	Vec2 inputDirection = Vec2::ZERO;
//
//	// WASD movement
//	if (g_theInput->IsKeyDown('W')) {
//		inputDirection.y += 1.0f;
//	}
//	if (g_theInput->IsKeyDown('S')) {
//		inputDirection.y -= 1.0f;
//	}
//	if (g_theInput->IsKeyDown('A')) {
//		inputDirection.x -= 1.0f;
//	}
//	if (g_theInput->IsKeyDown('D')) {
//		inputDirection.x += 1.0f;
//	}
//
//	// Normalize diagonal movement
//	if (inputDirection.GetLengthSquared() > 0.0f) {
//		inputDirection.Normalize();
//	}
//
//	// Set target velocity
//	Vec2 targetVelocity = inputDirection * m_moveSpeed;
//
//	// Store for movement update
//	// We'll interpolate to target velocity in UpdateMovement
//	m_velocity = targetVelocity;
//}
//
//void GamePlayer::HandleCameraZoomInput()
//{
//	float wheelDelta = g_theInput->GetMouseWheelDelta();
//
//	if (wheelDelta != 0.0f) {
//		float zoomChange = wheelDelta * m_zoomSpeed;
//		float newZoom = m_cameraZoom + zoomChange;
//		newZoom = GetClamped(newZoom, m_minZoom, m_maxZoom);
//		SetCameraZoom(newZoom);
//	}
//}
//
//void GamePlayer::HandleMatBrushInput()
//{
//	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
//	{
//		Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
//		Vec2 mousePosInWorld = AABB2(m_camera.GetOrthoBottomLeft(), m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);
//		int gridX = static_cast<int> (floor(mousePosInWorld.x));
//		int gridY = static_cast<int> (floor(mousePosInWorld.y));
//
//		if (m_gameMap->IsInBounds(gridX, gridY))
//		{
//			for (int dx = 0; dx < m_brushSize; dx++)
//			{
//				for (int dy = 0; dy < m_brushSize; dy++)
//				{
//					int targetX = gridX + dx;
//					int targetY = gridY + dy;
//					if (m_gameMap->IsInBounds(targetX, targetY))
//					{
//						m_gameMap->PlaceMaterialInChunk(targetX, targetY, m_selectedMaterial, false);
//					}
//				}
//			}
//		}
//	}
//}
//
//void GamePlayer::SetCameraZoom(float zoom)
//{
//	m_cameraZoom = GetClamped(zoom, m_minZoom, m_maxZoom);
//}
//
//void GamePlayer::SetZoomLimits(float minZoom, float maxZoom)
//{
//	m_minZoom = minZoom;
//	m_maxZoom = maxZoom;
//	m_cameraZoom = GetClamped(m_cameraZoom, m_minZoom, m_maxZoom);
//}
//
//AABB2 const& GamePlayer::GetBaseCamBound() const
//{
//	static AABB2 baseBound;
//	baseBound = AABB2(
//		m_position - Vec2(m_baseViewWidth * 0.5f, m_baseViewHeight * 0.5f),
//		m_position + Vec2(m_baseViewWidth * 0.5f, m_baseViewHeight * 0.5f)
//	);
//	return baseBound;
//}
//
//void GamePlayer::UpdateMovement(float deltaTime)
//{
//	// Simple direct movement (could add acceleration/deceleration if desired)
//	Vec2 movement = m_velocity * deltaTime;
//	m_position += movement;
//
//	// Optional: Clamp to map bounds if needed
//	if (m_gameMap) {
//		IntVec2 mapSize = m_gameMap->GetMapSize();
//		m_position.x = GetClamped(m_position.x, m_radius, (float)mapSize.x - m_radius);
//		m_position.y = GetClamped(m_position.y, m_radius, (float)mapSize.y - m_radius);
//	}
//}
//
//void GamePlayer::UpdateCamera()
//{
//	Vec2 cameraCenter = m_position;
//
//	// Use DIVISION to make zoom work correctly:
//	// Higher zoom = smaller view = zoomed IN
//	// Lower zoom = larger view = zoomed OUT
//	float viewWidth = m_baseViewWidth / m_cameraZoom;
//	float viewHeight = m_baseViewHeight / m_cameraZoom;
//
//	Vec2 newMins = cameraCenter - Vec2(viewWidth * 0.5f, viewHeight * 0.5f);
//	Vec2 newMaxs = cameraCenter + Vec2(viewWidth * 0.5f, viewHeight * 0.5f);
//	m_camera.SetOrthographicView(newMins, newMaxs,-1.f,100.f);
//}
//
//void GamePlayer::Render() const
//{
//	std::vector<Vertex_PCU> verts;
//
//	// Draw circle for player
//	const int numSegments = 32;
//	AddVertsForDisc2D(verts, m_position, m_radius, m_color);
//
//	// Draw direction indicator (small white dot in movement direction)
//	if (m_velocity.GetLengthSquared() > 0.01f) {
//		Vec2 normalizedVel = m_velocity.GetNormalized();
//		Vec2 indicatorPos = m_position + normalizedVel * (m_radius * 0.7f);
//		AddVertsForDisc2D(verts, indicatorPos, m_radius * 0.15f, Rgba8::WHITE);
//	}
//	AABB2 originalCamBound = AABB2(m_position - Vec2(m_baseViewWidth / 2.f, m_baseViewHeight / 2.f),
//		m_position + Vec2(m_baseViewWidth / 2.f, m_baseViewHeight / 2.f));
//	AddVertsForAABBWire2D(verts, originalCamBound, Rgba8::YELLOW, 5.f, false);
//
//	// Render
//	g_theRenderer->BindShader(nullptr);
//	g_theRenderer->BindTexture(nullptr);
//	g_theRenderer->SetModelConstants();
//	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
//	g_theRenderer->DrawVertexArray(verts);
//}
//
//void GamePlayer::RenderImgui()
//{
//	// Optional: Add ImGui debug info for player
//	// For now, keep it simple
//}
//
//void GamePlayer::InitCamera(IntVec2 const& viewSize)
//{
//	IntVec2 windowDimension = g_theWindow->GetClientDimensions();
//	m_camera.SetViewport(AABB2(Vec2(0.f, 0.f), Vec2((float)windowDimension.x, (float)windowDimension.y)));
//
//	// Calculate view size (10% larger than map size)
//	float viewWidth = (float)viewSize.x;
//	float viewHeight = (float)viewSize.y;
//
//	// Calculate view bounds centered on map
//	Vec2 viewMins = m_position - Vec2(viewWidth * 0.5f, viewHeight * 0.5f);
//	Vec2 viewMaxs = m_position + Vec2(viewWidth * 0.5f, viewHeight * 0.5f);
//
//	m_camera.SetOrthographicView(viewMins, viewMaxs ,- 1.0f, 100.0f);
//
//	m_baseViewWidth = viewMaxs.x - viewMins.x;
//	m_baseViewHeight = viewMaxs.y - viewMins.y;
//}
//
//void GamePlayer::SetPosition(Vec2 const& position)
//{
//	m_position = position;
//}

#include "GamePlayer.hpp"
#include "GameMap.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "SandboxMap.hpp"              // for CELLS_PER_METER / METERS_PER_CELL
#include <ThirdParty/box2d/include/box2d/box2d.h>
#include "Engine/Core/ErrorWarningAssert.hpp"

extern InputSystem* g_theInput;
extern Window* g_theWindow;
extern Renderer* g_theRenderer;

// ============================================================
//  Construction / Destruction
// ============================================================

GamePlayer::GamePlayer(IntVec2 const& mapSize, Vec2 const& startPosition)
	: m_position(startPosition)
	, m_velocity(Vec2::ZERO)
	, m_moveSpeed(200.0f)
	, m_acceleration(1000.0f)
	, m_deceleration(800.0f)
	, m_radius(13.0f)
	, m_color(Rgba8(255, 200, 0))
	, m_cameraZoom(1.0f)
	, m_minZoom(0.05f)
	, m_maxZoom(5.0f)
	, m_zoomSpeed(0.1f)
	, m_cameraCurrentPos(startPosition)
	, m_gameMap(nullptr)
{
	(void)mapSize;
	Texture* sprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerSheet.png");
	InitSprite(sprite, IntVec2(4, 4), 4, 14, 0, 3,12);
}

GamePlayer::~GamePlayer()
{
	DestroyPlayerBody();
	delete m_idleAnim;
	delete m_moveAnim;
	delete m_spriteSheet;
	// m_spriteSheet is not owned here
}

// ============================================================
//  Sprite Init
// ============================================================

void GamePlayer::InitSprite(Texture* texture, IntVec2 const& gridLayout,
	int idleStartIndex, int idleEndIndex,
	int moveStartIndex, int moveEndIndex,
	float fps)
{
	delete m_spriteSheet;
	m_spriteSheet = new SpriteSheet(*texture, gridLayout);

	delete m_idleAnim;
	delete m_moveAnim;
	m_idleAnim = new SpriteAnimDefinition(*m_spriteSheet, idleStartIndex, idleEndIndex, fps, SpriteAnimPlaybackType::LOOP);
	m_moveAnim = new SpriteAnimDefinition(*m_spriteSheet, moveStartIndex, moveEndIndex, fps, SpriteAnimPlaybackType::LOOP);
}

// ============================================================
//  Map Reference (creates body once world is available)
// ============================================================

void GamePlayer::SetGameMap(GameMap* map)
{
	m_gameMap = map;

	// Re-create the physics body against the new world
	DestroyPlayerBody();
	if (m_gameMap && m_physicsEnabled)
	{
		CreatePlayerBody();
	}
}

// ============================================================
//  Physics Body
// ============================================================

void GamePlayer::CreatePlayerBody()
{
	if (!m_gameMap) return;
	b2WorldId worldId = m_gameMap->GetB2WorldId();
	if (B2_IS_NULL(worldId)) return;

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.gravityScale = 1.0f;
	bodyDef.motionLocks.angularZ = false;
	bodyDef.position = { m_position.x* METERS_PER_CELL, m_position.y * METERS_PER_CELL };
	// fixedRotation不存在，kinematic body本身不会旋转，无需设置

	m_b2BodyId = b2CreateBody(worldId, &bodyDef);

	b2Circle circle;
	circle.center = { 0.f, 0.f };
	circle.radius = m_radius * METERS_PER_CELL;

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.material.friction = 0.0f;   // ← 正确字段名
	shapeDef.material.restitution = 0.0f;   // ← 正确字段名

	b2ShapeId shapeId = b2CreateCircleShape(m_b2BodyId, &shapeDef, &circle);
	DebuggerPrintf("Player circle shape valid: %d\n", b2Shape_IsValid(shapeId));

	// 完全不管输入，直接给一个向下的速度
	//b2Body_SetLinearVelocity(m_b2BodyId, { 0.f, -50.f });

	// 在CreatePlayerBody里，创建shape之后
	b2Filter playerFilter = b2Shape_GetFilter(shapeId);
	DebuggerPrintf("Player filter - category: %llu  mask: %llu  group: %d\n",
		playerFilter.categoryBits, playerFilter.maskBits, playerFilter.groupIndex);
}

void GamePlayer::DestroyPlayerBody()
{
	if (!B2_IS_NULL(m_b2BodyId))
	{
		b2DestroyBody(m_b2BodyId);
		m_b2BodyId = b2_nullBodyId;
	}
}

void GamePlayer::SetPhysicsEnabled(bool enabled)
{
	if (m_physicsEnabled == enabled) return;
	m_physicsEnabled = enabled;

	if (enabled)
	{
		CreatePlayerBody();
	}
	else
	{
		DestroyPlayerBody();
	}
}

// Push our desired velocity into the kinematic body each frame BEFORE the world step
void GamePlayer::SyncBodyToPosition()
{
	if (B2_IS_NULL(m_b2BodyId)) return;

	// Drive the kinematic body by setting its linear velocity (in metres/s)
	b2Vec2 b2vel = { m_velocity.x * METERS_PER_CELL , m_velocity.y* METERS_PER_CELL };
	DebuggerPrintf("Pushing velocity: %.2f, %.2f\n", b2vel.x, b2vel.y);
	b2Body_SetLinearVelocity(m_b2BodyId, b2vel);
}

// Pull the resolved position back after the world has stepped
void GamePlayer::SyncPositionFromBody()
{
	if (B2_IS_NULL(m_b2BodyId)) return;

	b2Vec2 pos = b2Body_GetPosition(m_b2BodyId);
	m_position.x = pos.x * CELLS_PER_METER;
	m_position.y = pos.y * CELLS_PER_METER;
}

// ============================================================
//  Core Update
// ============================================================

void GamePlayer::PrePhysicsUpdate(float deltaTime)
{
	(void)deltaTime;
	HandleInput();

	if (!B2_IS_NULL(m_b2BodyId))
	{
		SyncBodyToPosition();   // push desired velocity into Box2D before world step
	}
	else
	{
		// No physics body – integrate movement immediately
		UpdateMovement(deltaTime);
	}
}

void GamePlayer::PostPhysicsUpdate(float deltaTime)
{
	if (!B2_IS_NULL(m_b2BodyId))
	{
		b2Vec2 pos = b2Body_GetPosition(m_b2BodyId);
		b2Vec2 vel = b2Body_GetLinearVelocity(m_b2BodyId);
		DebuggerPrintf("After step - pos: %.2f, %.2f  vel: %.2f, %.2f\n",
			pos.x, pos.y, vel.x, vel.y);

		bool isEnabled = b2Body_IsEnabled(m_b2BodyId);
		DebuggerPrintf("Player body enabled: %d\n", isEnabled);
	}
	if (!B2_IS_NULL(m_b2BodyId))
	{
		SyncPositionFromBody(); // pull resolved position after world step
	}

	UpdateAnimation(deltaTime);
	UpdateCamera(deltaTime);
}

// Fallback for callers that don't split around a world step
void GamePlayer::Update(float deltaTime)
{
	PrePhysicsUpdate(deltaTime);
	// No world step here – position will lag one frame if physics is enabled,
	// but this keeps the interface functional outside of GameMap context.
	PostPhysicsUpdate(deltaTime);
}


// ============================================================
//  Input
// ============================================================

void GamePlayer::HandleInput()
{
	HandleMovementInput();
	HandleCameraZoomInput();
	HandleMatBrushInput();
}

void GamePlayer::HandleMovementInput()
{
	Vec2 inputDirection = Vec2::ZERO;

	if (g_theInput->IsKeyDown('W')) inputDirection.y += 1.0f;
	if (g_theInput->IsKeyDown('S')) inputDirection.y -= 1.0f;
	if (g_theInput->IsKeyDown('A')) inputDirection.x -= 1.0f;
	if (g_theInput->IsKeyDown('D')) inputDirection.x += 1.0f;

	if (inputDirection.GetLengthSquared() > 0.0f)
		inputDirection.Normalize();

	m_velocity = inputDirection * m_moveSpeed;

	// Track facing direction for sprite flip
	if (inputDirection.x > 0.01f)       m_facingRight = true;
	else if (inputDirection.x < -0.01f) m_facingRight = false;
}

void GamePlayer::HandleCameraZoomInput()
{
	float wheelDelta = g_theInput->GetMouseWheelDelta();
	if (wheelDelta != 0.0f)
	{
		float newZoom = m_cameraZoom + wheelDelta * m_zoomSpeed;
		SetCameraZoom(newZoom);
	}
}

void GamePlayer::HandleMatBrushInput()
{
	if (!g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE)) return;

	Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
	Vec2 mousePosInWorld = AABB2(m_camera.GetOrthoBottomLeft(), m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);
	int  gridX = static_cast<int>(floor(mousePosInWorld.x));
	int  gridY = static_cast<int>(floor(mousePosInWorld.y));

	if (!m_gameMap || !m_gameMap->IsInBounds(gridX, gridY)) return;

	for (int dx = 0; dx < m_brushSize; dx++)
	{
		for (int dy = 0; dy < m_brushSize; dy++)
		{
			int tx = gridX + dx;
			int ty = gridY + dy;
			if (m_gameMap->IsInBounds(tx, ty))
				m_gameMap->PlaceMaterialInChunk(tx, ty, m_selectedMaterial, false);
		}
	}
}

// ============================================================
//  Movement (used when physics body is absent)
// ============================================================

void GamePlayer::UpdateMovement(float deltaTime)
{
	m_position += m_velocity * deltaTime;

	if (m_gameMap)
	{
		IntVec2 mapSize = m_gameMap->GetMapSize();
		m_position.x = GetClamped(m_position.x, m_radius, (float)mapSize.x - m_radius);
		m_position.y = GetClamped(m_position.y, m_radius, (float)mapSize.y - m_radius);
	}
}

// ============================================================
//  Animation
// ============================================================

void GamePlayer::UpdateAnimation(float deltaTime)
{
	if (!m_spriteSheet) return;

	bool isMoving = m_velocity.GetLengthSquared() > 0.1f;

	PlayerAnimState newState = isMoving ? PlayerAnimState::MOVE : PlayerAnimState::IDLE;
	if (newState != m_animState)
	{
		m_animState = newState;
		m_animTimer = 0.0f;
	}

	m_animTimer += deltaTime;
}

// ============================================================
//  Camera
// ============================================================

void GamePlayer::SetCameraZoom(float zoom)
{
	m_cameraZoom = GetClamped(zoom, m_minZoom, m_maxZoom);
}

void GamePlayer::SetZoomLimits(float minZoom, float maxZoom)
{
	m_minZoom = minZoom;
	m_maxZoom = maxZoom;
	m_cameraZoom = GetClamped(m_cameraZoom, m_minZoom, m_maxZoom);
}

AABB2 const& GamePlayer::GetBaseCamBound() const
{
	static AABB2 baseBound;
	baseBound = AABB2(
		m_position - Vec2(m_baseViewWidth * 0.5f, m_baseViewHeight * 0.5f),
		m_position + Vec2(m_baseViewWidth * 0.5f, m_baseViewHeight * 0.5f)
	);
	return baseBound;
}

void GamePlayer::UpdateCamera(float deltaTime)
{
	// ── Smooth target position ────────────────────────────────
	if (m_cameraSmoothing)
	{
		// Exponential-decay lerp: framerate-independent
		float t = 1.0f - expf(-m_cameraSmoothSpeed * deltaTime);
		m_cameraCurrentPos.x = Interpolate(m_cameraCurrentPos.x, m_position.x, t);
		m_cameraCurrentPos.y = Interpolate(m_cameraCurrentPos.y, m_position.y, t);
	}
	else
	{
		m_cameraCurrentPos = m_position;
	}

	// ── Apply zoom ────────────────────────────────────────────
	float viewWidth = m_baseViewWidth / m_cameraZoom;
	float viewHeight = m_baseViewHeight / m_cameraZoom;

	Vec2 newMins = m_cameraCurrentPos - Vec2(viewWidth * 0.5f, viewHeight * 0.5f);
	Vec2 newMaxs = m_cameraCurrentPos + Vec2(viewWidth * 0.5f, viewHeight * 0.5f);
	m_camera.SetOrthographicView(newMins, newMaxs, -1.f, 100.f);
}

// ============================================================
//  Render
// ============================================================

void GamePlayer::Render() const
{
	std::vector<Vertex_PCU> verts;

	if (m_spriteSheet && m_idleAnim && m_moveAnim)
	{
		// ── Sprite rendering ─────────────────────────────────
		SpriteAnimDefinition* curAnim = (m_animState == PlayerAnimState::MOVE) ? m_moveAnim : m_idleAnim;
		SpriteDefinition const& spriteDef = curAnim->GetSpriteDefAtTime(m_animTimer);

		AABB2 uvs = m_facingRight ? spriteDef.GetUVs() : spriteDef.GetUVsReverse();

		// Draw sprite centred on player position.
		// Height = 2 * radius; width = height * aspect.
		float spriteHeight = m_radius * 6.0f;
		float spriteWidth = spriteHeight * spriteDef.GetAspect();

		AABB2 spriteBounds(
			m_position - Vec2(spriteWidth * 0.5f, spriteHeight * 0.5f),
			m_position + Vec2(spriteWidth * 0.5f, spriteHeight * 0.5f)
		);

		AddVertsForAABB2D(verts, spriteBounds, Rgba8::WHITE, uvs.m_mins, uvs.m_maxs);

		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(&spriteDef.GetTexture());
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->DrawVertexArray(verts);
		verts.clear();
	}
	else
	{
		// ── Fallback circle ───────────────────────────────────
		AddVertsForDisc2D(verts, m_position, m_radius, m_color);

		if (m_velocity.GetLengthSquared() > 0.01f)
		{
			Vec2 normalizedVel = m_velocity.GetNormalized();
			Vec2 indicatorPos = m_position + normalizedVel * (m_radius * 0.7f);
			AddVertsForDisc2D(verts, indicatorPos, m_radius * 0.15f, Rgba8::WHITE);
		}

		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->DrawVertexArray(verts);
		verts.clear();
	}

	// ── Base-cam bound debug outline ─────────────────────────
	AABB2 originalCamBound(
		m_position - Vec2(m_baseViewWidth * 0.5f, m_baseViewHeight * 0.5f),
		m_position + Vec2(m_baseViewWidth * 0.5f, m_baseViewHeight * 0.5f)
	);
	AddVertsForAABBWire2D(verts, originalCamBound, Rgba8::YELLOW, 5.f, false);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray(verts);
}

void GamePlayer::RenderImgui()
{
	// Optional debug UI
}

// ============================================================
//  Camera Init
// ============================================================

void GamePlayer::InitCamera(IntVec2 const& viewSize)
{
	IntVec2 windowDimension = g_theWindow->GetClientDimensions();
	m_camera.SetViewport(AABB2(Vec2(0.f, 0.f), Vec2((float)windowDimension.x, (float)windowDimension.y)));

	float viewWidth = (float)viewSize.x;
	float viewHeight = (float)viewSize.y;

	Vec2 viewMins = m_position - Vec2(viewWidth * 0.5f, viewHeight * 0.5f);
	Vec2 viewMaxs = m_position + Vec2(viewWidth * 0.5f, viewHeight * 0.5f);

	m_camera.SetOrthographicView(viewMins, viewMaxs, -1.0f, 100.0f);

	m_baseViewWidth = viewMaxs.x - viewMins.x;
	m_baseViewHeight = viewMaxs.y - viewMins.y;
	m_cameraCurrentPos = m_position;   // initialise smoother position
}

// ============================================================
//  Setters
// ============================================================

void GamePlayer::SetPosition(Vec2 const& position)
{
	m_position = position;

	// Also teleport the smoothed camera position so there is no slide on teleport
	m_cameraCurrentPos = position;

	// Teleport the Box2D body too
	if (!B2_IS_NULL(m_b2BodyId))
	{
		b2Body_SetTransform(m_b2BodyId,
			{ position.x * METERS_PER_CELL, position.y * METERS_PER_CELL },
			b2MakeRot(0.f));
	}
}