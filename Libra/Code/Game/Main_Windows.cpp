#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)
#include "Game/App.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Image.hpp"

extern App* g_theApp;

int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED( commandLineString );
	UNUSED(applicationInstanceHandle);

	Image testImage = Image("Data/Images/Terrain_8x8.png");

	g_theApp = new App();
	g_theApp->Startup();
	g_theApp->RunMainLoop();
	g_theApp->Shutdown();
	delete g_theApp;
	g_theApp = nullptr;
	return 0;
}


