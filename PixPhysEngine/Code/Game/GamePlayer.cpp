#include "GamePlayer.hpp"
#include "GameMap.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern InputSystem* g_theInput;
extern Window* g_theWindow;
extern Renderer* g_theRenderer;

GamePlayer::GamePlayer(IntVec2 const& mapSize, Vec2 const& startPosition)
	: //BasePlayer(mapSize)
	 m_position(startPosition)
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
	, m_gameMap(nullptr)
{

}

void GamePlayer::Update(float deltaTime)
{
	HandleInput();
	UpdateMovement(deltaTime);
	UpdateCamera();
}

void GamePlayer::HandleInput()
{
	HandleMovementInput();
	HandleCameraZoomInput();
	HandleMatBrushInput();
}

void GamePlayer::HandleMovementInput()
{
	Vec2 inputDirection = Vec2::ZERO;

	// WASD movement
	if (g_theInput->IsKeyDown('W')) {
		inputDirection.y += 1.0f;
	}
	if (g_theInput->IsKeyDown('S')) {
		inputDirection.y -= 1.0f;
	}
	if (g_theInput->IsKeyDown('A')) {
		inputDirection.x -= 1.0f;
	}
	if (g_theInput->IsKeyDown('D')) {
		inputDirection.x += 1.0f;
	}

	// Normalize diagonal movement
	if (inputDirection.GetLengthSquared() > 0.0f) {
		inputDirection.Normalize();
	}

	// Set target velocity
	Vec2 targetVelocity = inputDirection * m_moveSpeed;

	// Store for movement update
	// We'll interpolate to target velocity in UpdateMovement
	m_velocity = targetVelocity;
}

void GamePlayer::HandleCameraZoomInput()
{
	float wheelDelta = g_theInput->GetMouseWheelDelta();

	if (wheelDelta != 0.0f) {
		float zoomChange = wheelDelta * m_zoomSpeed;
		float newZoom = m_cameraZoom + zoomChange;
		newZoom = GetClamped(newZoom, m_minZoom, m_maxZoom);
		SetCameraZoom(newZoom);
	}
}

void GamePlayer::HandleMatBrushInput()
{
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
		Vec2 mousePosInWorld = AABB2(m_camera.GetOrthoBottomLeft(), m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);
		int gridX = static_cast<int> (floor(mousePosInWorld.x));
		int gridY = static_cast<int> (floor(mousePosInWorld.y));

		if (m_gameMap->IsInBounds(gridX, gridY))
		{
			for (int dx = 0; dx < m_brushSize; dx++)
			{
				for (int dy = 0; dy < m_brushSize; dy++)
				{
					int targetX = gridX + dx;
					int targetY = gridY + dy;
					if (m_gameMap->IsInBounds(targetX, targetY))
					{
						m_gameMap->PlaceMaterialInChunk(targetX, targetY, m_selectedMaterial, false);
					}
				}
			}
		}
	}
}

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

void GamePlayer::UpdateMovement(float deltaTime)
{
	// Simple direct movement (could add acceleration/deceleration if desired)
	Vec2 movement = m_velocity * deltaTime;
	m_position += movement;

	// Optional: Clamp to map bounds if needed
	if (m_gameMap) {
		IntVec2 mapSize = m_gameMap->GetMapSize();
		m_position.x = GetClamped(m_position.x, m_radius, (float)mapSize.x - m_radius);
		m_position.y = GetClamped(m_position.y, m_radius, (float)mapSize.y - m_radius);
	}
}

void GamePlayer::UpdateCamera()
{
	Vec2 cameraCenter = m_position;

	// Use DIVISION to make zoom work correctly:
	// Higher zoom = smaller view = zoomed IN
	// Lower zoom = larger view = zoomed OUT
	float viewWidth = m_baseViewWidth / m_cameraZoom;
	float viewHeight = m_baseViewHeight / m_cameraZoom;

	Vec2 newMins = cameraCenter - Vec2(viewWidth * 0.5f, viewHeight * 0.5f);
	Vec2 newMaxs = cameraCenter + Vec2(viewWidth * 0.5f, viewHeight * 0.5f);
	m_camera.SetOrthographicView(newMins, newMaxs,-1.f,100.f);
}

void GamePlayer::Render() const
{
	std::vector<Vertex_PCU> verts;

	// Draw circle for player
	const int numSegments = 32;
	AddVertsForDisc2D(verts, m_position, m_radius, m_color);

	// Draw direction indicator (small white dot in movement direction)
	if (m_velocity.GetLengthSquared() > 0.01f) {
		Vec2 normalizedVel = m_velocity.GetNormalized();
		Vec2 indicatorPos = m_position + normalizedVel * (m_radius * 0.7f);
		AddVertsForDisc2D(verts, indicatorPos, m_radius * 0.15f, Rgba8::WHITE);
	}
	AABB2 originalCamBound = AABB2(m_position - Vec2(m_baseViewWidth / 2.f, m_baseViewHeight / 2.f),
		m_position + Vec2(m_baseViewWidth / 2.f, m_baseViewHeight / 2.f));
	AddVertsForAABBWire2D(verts, originalCamBound, Rgba8::YELLOW, 5.f, false);

	// Render
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray(verts);
}

void GamePlayer::RenderImgui()
{
	// Optional: Add ImGui debug info for player
	// For now, keep it simple
}

void GamePlayer::InitCamera(IntVec2 const& viewSize)
{
	IntVec2 windowDimension = g_theWindow->GetClientDimensions();
	m_camera.SetViewport(AABB2(Vec2(0.f, 0.f), Vec2((float)windowDimension.x, (float)windowDimension.y)));

	// Calculate view size (10% larger than map size)
	float viewWidth = (float)viewSize.x;
	float viewHeight = (float)viewSize.y;

	// Calculate view bounds centered on map
	Vec2 viewMins = m_position - Vec2(viewWidth * 0.5f, viewHeight * 0.5f);
	Vec2 viewMaxs = m_position + Vec2(viewWidth * 0.5f, viewHeight * 0.5f);

	m_camera.SetOrthographicView(viewMins, viewMaxs ,- 1.0f, 100.0f);

	m_baseViewWidth = viewMaxs.x - viewMins.x;
	m_baseViewHeight = viewMaxs.y - viewMins.y;
}

void GamePlayer::SetPosition(Vec2 const& position)
{
	m_position = position;
}