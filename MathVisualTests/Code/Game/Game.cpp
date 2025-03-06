#include "Game/Game.hpp"
#include "App.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/RaycastAtrrow.hpp"

extern bool g_isDebugDraw;

Game::Game()
{
	
}
void Game::UpdateDeveloperCheats(float& deltaTime)
{
	AdjustForPauseAndTimeDitortion(deltaTime);
	if (g_theInput->WasKeyJustPressed('L'))
	{
		g_isDebugDraw = !g_isDebugDraw;
	}
}

void Game::UpdateKBInputForRaycastMode(float& deltaTime, RaycastArrow& arrow, float speed)
{
	if (g_theInput->IsKeyDown('E'))
	{
		arrow.m_startPos.y += speed * deltaTime;
	}

	if (g_theInput->IsKeyDown('S'))
	{
		arrow.m_startPos.x -= speed * deltaTime;
	}

	if (g_theInput->IsKeyDown('D'))
	{
		arrow.m_startPos.y -= speed * deltaTime;
	}

	if (g_theInput->IsKeyDown('F'))
	{
		arrow.m_startPos.x += speed * deltaTime;
	}

	if (g_theInput->IsKeyDown('I'))
	{
		arrow.m_endPos_whole.y += speed * deltaTime;
	}

	if (g_theInput->IsKeyDown('J'))
	{
		arrow.m_endPos_whole.x -= speed * deltaTime;
	}

	if (g_theInput->IsKeyDown('K'))
	{
		arrow.m_endPos_whole.y -= speed * deltaTime;
	}

	if (g_theInput->IsKeyDown('L'))
	{
		arrow.m_endPos_whole.x += speed * deltaTime;
	}

	if (g_theInput->IsKeyDown(KEYCODE_UPARROW))
	{
		arrow.m_startPos.y += speed * deltaTime;
		arrow.m_endPos_whole.y += speed * deltaTime;
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFTARROW))
	{
		arrow.m_startPos.x -= speed * deltaTime;
		arrow.m_endPos_whole.x -= speed * deltaTime;
	}

	if (g_theInput->IsKeyDown(KEYCODE_DOWNARROW))
	{
		arrow.m_startPos.y -= speed * deltaTime;
		arrow.m_endPos_whole.y -= speed * deltaTime;
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHTARROW))
	{
		arrow.m_startPos.x += speed * deltaTime;
		arrow.m_endPos_whole.x += speed * deltaTime;
	}
}



void Game::AdjustForPauseAndTimeDitortion(float& deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed('P'))
	{
		m_isPause = !m_isPause;
	}

	m_isSlow = g_theInput->IsKeyDown('T');

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPause = false;
		m_pauseAfterUpdate = true;
	}

	//--------------------------------------------------------------------------------------

	if (m_isPause)
	{
		deltaSeconds = 0.f;
	}
	if (m_isSlow)
	{
		deltaSeconds *= 0.10f;
	}
	if (m_pauseAfterUpdate)
	{
		m_isPause = true;
		m_pauseAfterUpdate = false;
	}
}












