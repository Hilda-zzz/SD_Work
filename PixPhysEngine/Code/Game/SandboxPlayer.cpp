#include "SandboxPlayer.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "SandboxMap.hpp"
extern Window* g_theWindow;
extern InputSystem* g_theInput;

SandboxPlayer::SandboxPlayer(IntVec2 const& mapSize)
{
	InitCamera(mapSize);
}

void SandboxPlayer::Update(float deltaTime)
{
	HandleInput();
}

void SandboxPlayer::Render() const
{
}

void SandboxPlayer::InitCamera(IntVec2 const& mapSize)
{
	IntVec2 windowDimension = g_theWindow->GetClientDimensions();
	m_camera.SetViewport(AABB2(Vec2(0.f, 0.f), Vec2((float)windowDimension.x, (float)windowDimension.y)));

	// Calculate map center
	Vec2 mapCenter = Vec2((float)mapSize.x * 0.5f, (float)mapSize.y * 0.5f);

	// Calculate view size (10% larger than map size)
	float viewWidth = (float)mapSize.x * 1.1f;
	float viewHeight = (float)mapSize.y * 1.1f;

	// Calculate view bounds centered on map
	Vec2 viewMins = mapCenter - Vec2(viewWidth * 0.5f, viewHeight * 0.5f);
	Vec2 viewMaxs = mapCenter + Vec2(viewWidth * 0.5f, viewHeight * 0.5f);

	m_camera.SetOrthographicView(viewMins, viewMaxs);
}

void SandboxPlayer::HandleInput()
{
	if (g_theInput->WasKeyJustPressed('C'))
	{
		m_isSand = !m_isSand;
	}
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
		Vec2 mousePosInWorld = AABB2(m_camera.GetOrthoBottomLeft(), m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);
		int gridX = floor(mousePosInWorld.x);
		int gridY = floor(mousePosInWorld.y);

		if (m_curMap->IsValidPosition(gridX, gridY))
		{
			if (m_isSand)
			{
				m_curMap->PlaceMaterial(gridX, gridY, CellMatType::MAT_SAND, 1);
				if (m_curMap->IsValidPosition(gridX + 1, gridY))
					m_curMap->PlaceMaterial(gridX + 1, gridY, CellMatType::MAT_SAND, 1);
				if (m_curMap->IsValidPosition(gridX - 1, gridY))
					m_curMap->PlaceMaterial(gridX - 1, gridY, CellMatType::MAT_SAND, 1);
				if (m_curMap->IsValidPosition(gridX, gridY + 1))
					m_curMap->PlaceMaterial(gridX, gridY + 1, CellMatType::MAT_SAND, 1);
				if (m_curMap->IsValidPosition(gridX, gridY - 1))
					m_curMap->PlaceMaterial(gridX, gridY - 1, CellMatType::MAT_SAND, 1);
			}
			else
				m_curMap->PlaceMaterial(gridX, gridY, CellMatType::MAT_WATER, 1);
		}
	}
}

Vec2 SandboxPlayer::GetMouseWorldPosition() const
{
	return Vec2();
}

bool SandboxPlayer::IsPlacing() const
{
	return false;
}

bool SandboxPlayer::IsErasing() const
{
	return false;
}
