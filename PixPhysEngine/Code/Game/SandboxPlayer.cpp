#include "SandboxPlayer.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "SandboxMap.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "SandboxUI.hpp"
#include <memory>
extern Window* g_theWindow;
extern InputSystem* g_theInput;

SandboxPlayer::SandboxPlayer(IntVec2 const& mapSize) //: BasePlayer(mapSize)
{
	//InitCamera(mapSize);
	m_sandBoxUI = SandBoxUI();
	m_rigidBodyDrawnCells.reserve(2000);
}

void SandboxPlayer::Update(float deltaTime)
{
	UNUSED(deltaTime);
	HandleInput();
}

void SandboxPlayer::RenderImgui()
{
	m_sandBoxUI.RenderMaterialBrushUI(this);
	m_sandBoxUI.RenderRigidBodyPanel(this);
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
	if (!m_sandBoxUI.IsInRigidBodyDrawMode())
	{
		//if (g_theInput->WasKeyJustPressed('C'))
		//{
		//	m_isSand = !m_isSand;
		//}
		if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
		{
			Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
			Vec2 mousePosInWorld = AABB2(m_camera.GetOrthoBottomLeft(), m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);
			int gridX = static_cast<int> (floor(mousePosInWorld.x));
			int gridY = static_cast<int> (floor(mousePosInWorld.y));

			if (m_curMap->IsInBounds(gridX, gridY))
			{
				for (int dx = 0; dx < m_brushSize; dx++)
				{
					for (int dy = 0; dy < m_brushSize; dy++)
					{
						int targetX = gridX + dx;
						int targetY = gridY + dy;
						if (m_curMap->IsInBounds(targetX, targetY))
						{
							dynamic_cast<SandboxMap*>(m_curMap)->PlaceMaterialInChunk(targetX, targetY, m_selectedMaterial, false);
						}
					}
				}
			}
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
		{
			// Start dragging
			m_isRightMouseDragging = true;
			Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
			Vec2 mousePosInWorld = AABB2(m_camera.GetOrthoBottomLeft(), m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);
			m_dragStartX = static_cast<int>(floor(mousePosInWorld.x));
			m_dragStartY = static_cast<int>(floor(mousePosInWorld.y));
		}

		if (g_theInput->IsKeyDown(KEYCODE_RIGHT_MOUSE) && m_isRightMouseDragging)
		{
			// Update current drag position
			Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
			Vec2 mousePosInWorld = AABB2(m_camera.GetOrthoBottomLeft(), m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);
			m_dragCurrentX = static_cast<int>(floor(mousePosInWorld.x));
			m_dragCurrentY = static_cast<int>(floor(mousePosInWorld.y));
		}

		if (g_theInput->WasKeyJustReleased(KEYCODE_RIGHT_MOUSE) && m_isRightMouseDragging)
		{
			// Finish dragging and fill rectangle with stone
			m_isRightMouseDragging = false;

			Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
			Vec2 mousePosInWorld = AABB2(m_camera.GetOrthoBottomLeft(), m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);
			int endX = static_cast<int>(floor(mousePosInWorld.x));
			int endY = static_cast<int>(floor(mousePosInWorld.y));

			// Calculate rectangle bounds
			int minX = (m_dragStartX < endX) ? m_dragStartX : endX;
			int maxX = (m_dragStartX > endX) ? m_dragStartX : endX;
			int minY = (m_dragStartY < endY) ? m_dragStartY : endY;
			int maxY = (m_dragStartY > endY) ? m_dragStartY : endY;

			// Fill rectangle with stone
			for (int x = minX; x <= maxX; x++)
			{
				for (int y = minY; y <= maxY; y++)
				{
					if (m_curMap->IsInBounds(x, y))
					{
						dynamic_cast<SandboxMap*>(m_curMap)->PlaceMaterialInChunk(x, y, CellMatType::MAT_STONE, false);
					}
				}
			}
		}
	}
	else
	{
		if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
		{
			Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
			Vec2 mousePosInWorld = AABB2(m_camera.GetOrthoBottomLeft(), m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);
			int gridX = static_cast<int> (floor(mousePosInWorld.x));
			int gridY = static_cast<int> (floor(mousePosInWorld.y));

			if (m_curMap->IsInBounds(gridX, gridY))
			{
				for (int dx = 0; dx <= 4; dx++)
				{
					for (int dy = 0; dy <= 4; dy++)
					{
						int targetX = gridX + dx;
						int targetY = gridY + dy;
						if (m_curMap->IsInBounds(targetX, targetY)&&m_curMap->GetCell(targetX,targetY).IsEmpty())
						{
							dynamic_cast<SandboxMap*>(m_curMap)->PlaceMaterialInChunk(targetX, targetY, m_selectedMaterial, true);
							CellWithCoords cellWithCoords = CellWithCoords(&m_curMap->GetCell(targetX, targetY), IntVec2(targetX, targetY));
							m_rigidBodyDrawnCells.push_back(cellWithCoords);
						}
					}
				}
			}
		}
	}
}

Vec2 SandboxPlayer::GetMouseWorldPosition() const
{
	return Vec2();
}

//bool SandboxPlayer::IsPlacing() const
//{
//	return false;
//}
//
//bool SandboxPlayer::IsErasing() const
//{
//	return false;
//}

void SandboxPlayer::ClearRigidBodyDrawnCells()
{
	m_rigidBodyDrawnCells.clear();
}

