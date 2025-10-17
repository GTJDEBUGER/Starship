#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"

#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"

#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)
#include <cmath>
#include <cassert>
#include <crtdbg.h>

//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Later we will move this useful macro to a more central place, e.g. Engine/Core/EngineCommon.hpp
//
#define UNUSED(x) (void)(x);


//-----------------------------------------------------------------------------------------------
// #SD1ToDo: This will eventually go away once we add a Window engine class later on.
// 
constexpr float CLIENT_ASPECT = 2.f; // By the Assignment 1 requirements, this should be 2:1 aspect ratio


//-----------------------------------------------------------------------------------------------
extern HDC g_displayDeviceContext = nullptr;


//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED( applicationInstanceHandle);
	UNUSED( commandLineString );

	g_app = new App();

	while (!g_app->IsQuitting())
	{
		g_app->RunFrame();
	}

	delete g_app;
	g_app = nullptr;

	return 0;
}


